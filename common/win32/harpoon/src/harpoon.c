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
HHOOK hook_handles[WH_MAX+1];
BOOL block_input = FALSE;
HWND unblocked_windows[HARPOON_MAX_UNBLOCKED_WINDOWS];
#pragma data_seg()

static HANDLE dll_handle = NULL;
static volatile HOOKPROC hook_user_callbacks[WH_MAX+1];
static HOOKPROC hook_impl_callbacks[WH_MAX+1];
static ATOM notification_class = 0;

typedef struct
{
  MOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
  DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;

typedef struct
{
  int code;
  WPARAM wparam;
  LPARAM lparam;
} HarpoonMessage;

typedef struct
{
  HarpoonMessage message;
  MOUSEHOOKSTRUCTEX mouse;
} HarpoonMouseMessage;

typedef struct
{
  HarpoonMessage message;
  KBDLLHOOKSTRUCT keyboard_ll;
} HarpoonKeyboardLLMessage;


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
harpoon_post_message (int hook, HarpoonMessage *msg, DWORD msg_size)
{
  COPYDATASTRUCT copy;
  WPARAM wparam;
  
  copy.lpData = msg;
  copy.dwData = hook;
  if (hook == WH_MOUSE)
    {
      wparam = (WPARAM) ((HarpoonMouseMessage *) msg)
        ->mouse.MOUSEHOOKSTRUCT.hwnd;
    }
  else
    {
      wparam = 0;
    }
  copy.cbData = msg_size;
  SendMessage (notification_window, WM_COPYDATA, wparam, (LPARAM) &copy);
}

static LRESULT CALLBACK
harpoon_window_proc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_COPYDATA)
    {
      PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
      HarpoonMessage *msg = (HarpoonMessage *) copy->lpData;
      DWORD hook_id = copy->dwData;
      switch (hook_id)
	{
	case WH_MOUSE:
	  {
            HOOKPROC proc;
            HarpoonMouseMessage *mmsg = (HarpoonMouseMessage *) msg;
            
            proc = hook_user_callbacks[WH_MOUSE];
            if (proc != NULL)
              {
                (*proc)(msg->code, msg->wparam, (LPARAM) &mmsg->mouse);
              }
	    break;
	  }

	case WH_KEYBOARD_LL:
	  {
            HOOKPROC proc;
            HarpoonKeyboardLLMessage *kmsg = (HarpoonKeyboardLLMessage *) msg;
            
            proc = hook_user_callbacks[WH_KEYBOARD_LL];
            if (proc != NULL)
              {
                (*proc)(msg->code, msg->wparam, (LPARAM) &kmsg->keyboard_ll);
              }
	    break;
	  }

        default:
          {
            HOOKPROC proc;
            proc = hook_user_callbacks[hook_id];
            if (proc != NULL)
              {
                (*proc)(msg->code, msg->wparam, msg->lparam);
              }
	  }
          break;

        }
    }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



/**********************************************************************
 * Generic hook
 **********************************************************************/


static LRESULT
harpoon_generic_hook_return(int code, WPARAM wpar, LPARAM lpar, int hid)
{
  BOOL blocked = FALSE;
  LRESULT ret;
  
  if (block_input && code == HC_ACTION)
    {
      HWND target_window;
      if (hid == WH_MOUSE)
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
      ret = CallNextHookEx(hook_handles[hid], code, wpar, lpar);
    }
  return ret;
}

static LRESULT CALLBACK 
harpoon_generic_hook(int code, WPARAM wpar, LPARAM lpar, int hid)
{
  HarpoonMessage msg;

  msg.code = code;
  msg.wparam = wpar;
  msg.lparam = lpar;
  harpoon_post_message (hid, &msg, sizeof(msg));
  return harpoon_generic_hook_return (code, wpar, lpar, hid);
}
  

static void
harpoon_unhook_by_id(int hook_id)
{
  HHOOK handle = hook_handles[hook_id];
  if (handle != NULL)
    {
      UnhookWindowsHookEx(handle);
      hook_handles[hook_id] = NULL;
      hook_user_callbacks[hook_id] = NULL;
    }
}

void
harpoon_unhook(HHOOK handle)
{
  int hook_id;
  for (hook_id = 0; hook_id <= WH_MAX; hook_id++)
    {
      if (hook_handles[hook_id] == handle)
        {
          harpoon_unhook_by_id(hook_id);
          break;
        }
    }
}

HHOOK
harpoon_hook(int hook_id, HOOKPROC hf)
{
  HHOOK ret = NULL;
  if (hook_id >= 0 && hook_id <= WH_MAX 
      && hook_handles[hook_id] == NULL
      && hook_impl_callbacks[hook_id] != NULL)
    {
      ret = hook_handles[hook_id] = SetWindowsHookEx(hook_id, hook_impl_callbacks[hook_id], dll_handle, 0);
      if (ret != NULL)
        {
          hook_user_callbacks[hook_id] = hf;
        }
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
  HarpoonMouseMessage msg;
  PMOUSEHOOKSTRUCT pmhs;

  msg.message.code = code;
  msg.message.wparam = wpar;
  msg.message.lparam = 0;
  
  pmhs = (PMOUSEHOOKSTRUCT) lpar;
  msg.mouse.MOUSEHOOKSTRUCT = *pmhs;
  msg.mouse.mouseData = 0;

  if (harpoon_supports_mouse_hook_struct_ex())
    {
      msg.mouse.mouseData = ((PMOUSEHOOKSTRUCTEX) pmhs)->mouseData;
    }

  harpoon_post_message (WH_MOUSE, &msg.message, sizeof(msg));

  return harpoon_generic_hook_return (code, wpar, lpar, WH_MOUSE);
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
  return harpoon_generic_hook (code, wpar, lpar, WH_KEYBOARD);
}


static LRESULT CALLBACK 
harpoon_keyboard_ll_hook (int code, WPARAM wpar, LPARAM lpar)
{
  HarpoonKeyboardLLMessage msg;

  msg.message.code = code;
  msg.message.wparam = wpar;
  msg.message.lparam = 0;
  
  msg.keyboard_ll = *((KBDLLHOOKSTRUCT *) lpar);
  harpoon_post_message (WH_KEYBOARD_LL, &msg.message, sizeof(msg));

  return harpoon_generic_hook_return (code, wpar, lpar, WH_KEYBOARD_LL);
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

  memset(hook_user_callbacks, 0, sizeof(hook_user_callbacks));
  memset(hook_impl_callbacks, 0, sizeof(hook_impl_callbacks));
  hook_impl_callbacks[WH_MOUSE] = harpoon_mouse_hook;
  hook_impl_callbacks[WH_KEYBOARD] = harpoon_keyboard_hook;
  hook_impl_callbacks[WH_KEYBOARD_LL] = harpoon_keyboard_ll_hook;
  memset(hook_handles, 0, sizeof(hook_handles));

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
  int hook_id;
  for (hook_id = 0; hook_id <= WH_MAX; hook_id++)
    {
      harpoon_unhook_by_id(hook_id);
    }

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

