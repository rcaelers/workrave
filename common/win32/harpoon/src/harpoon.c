/*
 * harpoon.c
 *
 * Copyright (C) 2002-2003 Raymond Penners <raymond@dotsphinx.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * $Id$
 */

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>

#include "harpoon.h"

#ifndef WH_KEYBOARD_LL
#error WH_KEYBOARD_LL not defined.
#endif


#define HARPOON_MAX_UNBLOCKED_WINDOWS 16 /* Fixed, but ought to be enough... */
#define HARPOON_WINDOW_CLASS "HarpoonNotificationWindow"

#pragma comment(linker, "/SECTION:.shared,RWS")
#pragma data_seg(".shared")
HWND notification_window = NULL;
HHOOK mouse_hook = NULL;
HHOOK keyboard_hook = NULL;
HHOOK keyboard_ll_hook = NULL;
BOOL block_input = FALSE;
HWND unblocked_windows[HARPOON_MAX_UNBLOCKED_WINDOWS];
#pragma data_seg()

static HANDLE dll_handle = NULL;
static volatile HarpoonHookFunc user_callback = NULL;
static ATOM notification_class = 0;

typedef struct
{
  MOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
  DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;



/**********************************************************************
 * Misc
 **********************************************************************/

#ifdef GRAVEYARD

static DWORD
harpoon_get_service_pack()
{
  HKEY hKey;
  DWORD dwCSDVersion;
  DWORD dwSize;
  DWORD ret = 0;
 
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
      "System\\CurrentControlSet\\Control\\Windows",
       0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
      dwSize = sizeof(dwCSDVersion);
      if (RegQueryValueEx(hKey, "CSDVersion", NULL, NULL,
                          (unsigned char*)&dwCSDVersion, 
                           &dwSize) == ERROR_SUCCESS)
        {
          ret = (LOWORD(dwCSDVersion));
        }
      RegCloseKey(hKey);
    }
  return ret;
}           

#endif


/**********************************************************************
 * Blocking
 **********************************************************************/

static BOOL
harpoon_is_window_blocked (HWND hwnd)
{
  BOOL ret;

  ret = block_input;
  if (ret && hwnd != NULL)
    {
      int i;

      for (i = 0; i < HARPOON_MAX_UNBLOCKED_WINDOWS; i++)
        {
          HWND ubw = unblocked_windows[i];
          if (ubw == NULL)
            break;
          /* FIXME: GetParent is not enough, traverse all ancestors. */
          if (hwnd == ubw || GetParent (hwnd) == ubw)
            {
              ret = FALSE;
              break;
            }
        }
    }
  return ret;
}

HARPOON_API void
harpoon_unblock_input (void)
{
  block_input = FALSE;
}

HARPOON_API void
harpoon_block_input_except_for (HWND *unblocked)
{
  int i;
  BOOL last;
  
  block_input = TRUE;
  last = FALSE;
  for (i = 0; i < HARPOON_MAX_UNBLOCKED_WINDOWS; i++)
    {
      HWND hwnd;
      if (! last)
        {
          hwnd = unblocked[i];
          last = (hwnd == NULL);
        }
      else
        {
          hwnd = NULL;
        }
      unblocked_windows[i] = hwnd;
    }
}



/**********************************************************************
 * Messaging
 **********************************************************************/

static void
harpoon_post_message(HarpoonEventType evt, int par1, int par2)
{
  PostMessage (notification_window, WM_USER + evt, (WPARAM) par1, (LPARAM) par2);
}

static LRESULT CALLBACK
harpoon_window_proc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (user_callback)
    {
      HarpoonEvent evt;
      int evt_type;
      evt.type = HARPOON_NOTHING;
      evt_type = uMsg - WM_USER;
      if (evt_type >= 0 && evt_type < HARPOON_EVENT__SIZEOF)
        {
          evt.type = (HarpoonEventType) evt_type;
          switch (evt.type)
            {
            case HARPOON_KEY_PRESS:
            case HARPOON_KEY_RELEASE:
              evt.keyboard.flags = lParam;
              break;

            case HARPOON_BUTTON_PRESS:
            case HARPOON_BUTTON_RELEASE:
            case HARPOON_2BUTTON_PRESS:
            case HARPOON_MOUSE_WHEEL:
            case HARPOON_MOUSE_MOVE:
              evt.mouse.x = LOWORD(lParam);
              evt.mouse.y = HIWORD(lParam);
              if (evt.type == HARPOON_MOUSE_WHEEL)
                {
                  evt.mouse.button = -1;
                  evt.mouse.wheel = wParam;
                }
              else
                {
                  evt.mouse.button = wParam;
                  evt.mouse.wheel = 0;
                }
              break;

            default:
              evt.type = HARPOON_NOTHING;
            }

          if (evt.type != HARPOON_NOTHING)
            {
              (*user_callback)(&evt);
            }
        }

    }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



/**********************************************************************
 * Generic hook
 **********************************************************************/


static LRESULT
harpoon_generic_hook_return(int code, WPARAM wpar, LPARAM lpar, HHOOK hook)
{
  BOOL blocked = FALSE;
  LRESULT ret;
  
  if (block_input && code == HC_ACTION)
    {
      HWND target_window;
      if (hook == mouse_hook)
        {
          PMOUSEHOOKSTRUCT pmhs = (PMOUSEHOOKSTRUCT) lpar;
          target_window = pmhs->hwnd;
        }
      else
        {
          target_window = NULL;
        }
      blocked = harpoon_is_window_blocked(target_window);
    }
  if (blocked)
    {
      ret = -1;
    }
  else
    {
      ret = CallNextHookEx(hook, code, wpar, lpar);
    }
  return ret;
}




/**********************************************************************
 * Mouse hook
 **********************************************************************/

static BOOL
harpoon_supports_mouse_hook_struct_ex(void)
{
  DWORD dwVersion = GetVersion ();
  DWORD dwWindowsMajorVersion =  (DWORD) (LOBYTE(LOWORD(dwVersion)));
  return (dwWindowsMajorVersion >= 5);
}

LRESULT CALLBACK
harpoon_mouse_hook (int code, WPARAM wpar, LPARAM lpar)
{
  if (code == HC_ACTION)
    {
      PMOUSEHOOKSTRUCT pmhs = (PMOUSEHOOKSTRUCT) lpar;
      DWORD mouse_data = 0;
      HarpoonEventType evt = HARPOON_NOTHING;
      int button = -1;
      int x = pmhs->pt.x;
      int y = pmhs->pt.y;

      if (harpoon_supports_mouse_hook_struct_ex())
        {
          mouse_data = ((PMOUSEHOOKSTRUCTEX) pmhs)->mouseData;
        }


      switch (wpar)
        {
        case WM_LBUTTONDOWN:
          button = 0;
          evt = HARPOON_BUTTON_PRESS;
          break;
        case WM_MBUTTONDOWN:
          button = 1;
          evt = HARPOON_BUTTON_PRESS;
          break;
        case WM_RBUTTONDOWN:
          button = 2;
          evt = HARPOON_BUTTON_PRESS;
          break;
  
        case WM_LBUTTONUP:
          button = 0;
          evt = HARPOON_BUTTON_RELEASE;
          break;
        case WM_MBUTTONUP:
          button = 1;
          evt = HARPOON_BUTTON_RELEASE;
          break;
        case WM_RBUTTONUP:
          button = 2;
          evt = HARPOON_BUTTON_RELEASE;
          break;

        case WM_LBUTTONDBLCLK:
          button = 0;
          evt = HARPOON_2BUTTON_PRESS;
          break;
        case WM_MBUTTONDBLCLK:
          button = 1;
          evt = HARPOON_2BUTTON_PRESS;
          break;
        case WM_RBUTTONDBLCLK:
          button = 2;
          evt = HARPOON_2BUTTON_PRESS;
          break;

        case WM_MOUSEWHEEL:
          evt = HARPOON_MOUSE_WHEEL;
          button = mouse_data;
          break;

        default:
          evt = HARPOON_MOUSE_MOVE;
        }
      harpoon_post_message (evt, button, MAKELONG(x, y));
    }
  return harpoon_generic_hook_return (code, wpar, lpar, mouse_hook);
}




/**********************************************************************
 * Keyboard hook
 **********************************************************************/

#ifdef GRAVEYARD
static BOOL
harpoon_supports_keyboard_ll(void)
{
  OSVERSIONINFO info;
  BOOL ret = FALSE;
  if (GetVersionEx (&info))
    {
      if (info.dwPlatformId > VER_PLATFORM_WIN32_NT)
        {
          ret = TRUE;
        }
      else if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
          /* Check for min. SP3. */
          DWORD sp = harpoon_get_service_pack();
          ret = (sp >= 0x300);
        }

    }
  return ret;
}
#endif

static LRESULT CALLBACK 
harpoon_keyboard_hook (int code, WPARAM wpar, LPARAM lpar)
{
  if (code == HC_ACTION)
    {
      BOOL pressed = (lpar & (1 << 31)) == 0;
      BOOL prevpressed = (lpar & (1 << 30)) != 0;
      HarpoonEventType evt;
      int flags = 0;
      if (pressed && prevpressed)
        {
          flags |= HARPOON_KEY_REPEAT_FLAG;
        }

      evt = pressed ? HARPOON_KEY_PRESS : HARPOON_KEY_RELEASE;
      harpoon_post_message (evt, 0, flags);
    }
  return harpoon_generic_hook_return (code, wpar, lpar, keyboard_hook);
}


static LRESULT CALLBACK 
harpoon_keyboard_ll_hook (int code, WPARAM wpar, LPARAM lpar)
{
  if (code == HC_ACTION)
    {
      KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *) lpar;
      BOOL pressed = !(kb->flags & (1<<7));
      HarpoonEventType evt = pressed ? HARPOON_KEY_PRESS : HARPOON_KEY_RELEASE;
      PostMessage (notification_window, WM_USER + evt, (WPARAM) 0, (LPARAM) 0);
    }
  return harpoon_generic_hook_return (code, wpar, lpar, keyboard_ll_hook);
}


/**********************************************************************
 * Initialisation
 **********************************************************************/

HARPOON_API BOOL
harpoon_init (void)
{
  BOOL rc = FALSE;
  int i;
  WNDCLASSEX wclass =
  {
    sizeof(WNDCLASSEX),
    0,
    harpoon_window_proc,
    0,
    0,
    dll_handle,
    NULL,
    NULL,
    NULL,
    NULL,
    HARPOON_WINDOW_CLASS,
    NULL
  };

  block_input = FALSE;
  for (i = 0; i < HARPOON_MAX_UNBLOCKED_WINDOWS; i++)
    {
      unblocked_windows[i] = NULL;
    }


  notification_class = RegisterClassEx(&wclass);
  if (notification_class)
    {
      notification_window = CreateWindowEx
        (WS_EX_TOOLWINDOW, HARPOON_WINDOW_CLASS,
         HARPOON_WINDOW_CLASS, WS_OVERLAPPED,
         CW_USEDEFAULT, CW_USEDEFAULT,
         CW_USEDEFAULT, CW_USEDEFAULT,
         (HWND)NULL, (HMENU)NULL,
         dll_handle, (LPSTR)NULL);
      if (notification_window)
        {
          rc = TRUE;
        }
    }

  if (! rc)
    {
      harpoon_exit ();
    }
  
  return rc;
}

HARPOON_API void
harpoon_exit (void)
{
  harpoon_unhook();

  if (notification_window)
    {
      CloseWindow (notification_window);
      notification_window = NULL;
    }
  if (notification_class)
    {
      UnregisterClass (HARPOON_WINDOW_CLASS, dll_handle);
      notification_class = 0;
    }
}


void
harpoon_unhook ()
{
  if (mouse_hook)
    {
      UnhookWindowsHookEx(mouse_hook);
      mouse_hook = NULL;
    }
  if (keyboard_hook)
    {
      UnhookWindowsHookEx(keyboard_hook);
      keyboard_hook = NULL;
    }
  if (keyboard_ll_hook)
    {
      UnhookWindowsHookEx(keyboard_ll_hook);
      keyboard_ll_hook = NULL;
    }
  user_callback = NULL;
}

void 
harpoon_hook (HarpoonHookFunc func)
{
  harpoon_unhook();
  user_callback = func;
  mouse_hook = SetWindowsHookEx(WH_MOUSE, harpoon_mouse_hook, dll_handle, 0);
  keyboard_ll_hook = SetWindowsHookEx(WH_KEYBOARD_LL, harpoon_keyboard_ll_hook, dll_handle, 0);
  if (keyboard_ll_hook == NULL)
    {
      keyboard_hook = SetWindowsHookEx(WH_KEYBOARD, harpoon_keyboard_hook, dll_handle, 0);
    }

}



/**********************************************************************
 * Main
 **********************************************************************/

BOOL APIENTRY 
DllMain (HANDLE hModule, 
	 DWORD  ul_reason_for_call, 
	 LPVOID lpReserved)
{
  dll_handle = hModule;
  switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    }
  return TRUE;
}

