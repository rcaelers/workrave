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

#define HARPOON_MAX_UNBLOCKED_WINDOWS 16 /* Fixed, but ought to be enough... */
#define HARPOON_WINDOW_CLASS "HarpoonNotificationWindow"

#pragma comment(linker, "/SECTION:.shared,RWS")
#pragma data_seg(".shared")
HWND notification_window = NULL;
HHOOK mouse_hook_handle = NULL;
HHOOK keyboard_hook_handle = NULL;
BOOL block_input = FALSE;
HWND unblocked_windows[HARPOON_MAX_UNBLOCKED_WINDOWS];
#pragma data_seg()

static HANDLE dll_handle = NULL;
static volatile HOOKPROC mouse_hook_callback = NULL;
static volatile HOOKPROC keyboard_hook_callback = NULL;
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


/**********************************************************************
 * Misc
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
harpoon_post_message (int hook, HarpoonMessage *msg)
{
  COPYDATASTRUCT copy;
  WPARAM wparam;
  
  copy.lpData = msg;
  copy.dwData = hook;
  if (hook == WH_MOUSE)
    {
      wparam = (WPARAM) ((HarpoonMouseMessage *) msg)
        ->mouse.MOUSEHOOKSTRUCT.hwnd;
      copy.cbData = sizeof(HarpoonMouseMessage);
    }
  else
    {
      wparam = 0;
      copy.cbData = sizeof(HarpoonMessage);
    }
  SendMessage (notification_window, WM_COPYDATA, wparam, (LPARAM) &copy);
}

static LRESULT CALLBACK
harpoon_window_proc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_COPYDATA)
    {
      PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
      HarpoonMessage *msg = (HarpoonMessage *) copy->lpData;
      
      switch (copy->dwData)
	{
	case WH_KEYBOARD:
          {
            HOOKPROC proc;
            proc = keyboard_hook_callback;
            if (proc != NULL)
              {
                (*proc)(msg->code, msg->wparam, msg->lparam);
              }
	  }
          break;

	case WH_MOUSE:
	  {
            HOOKPROC proc;
            HarpoonMouseMessage *mmsg = (HarpoonMouseMessage *) msg;
            
            proc = mouse_hook_callback;
            if (proc != NULL)
              {
                (*proc)(msg->code, msg->wparam, (LPARAM) &mmsg->mouse);
              }
	    break;
	  }
        }
    }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
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
 * Generic hook
 **********************************************************************/


static LRESULT
harpoon_generic_hook_return(int code, WPARAM wpar, LPARAM lpar, HHOOK hhook)
{
  BOOL blocked = FALSE;
  LRESULT ret;
  
  if (block_input && code == HC_ACTION)
    {
      HWND target_window;
      if (hhook == mouse_hook_handle)
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
      ret = CallNextHookEx(hhook, code, wpar, lpar);
    }
  return ret;
}

static LRESULT CALLBACK 
harpoon_generic_hook(int code, WPARAM wpar, LPARAM lpar, int hid, HHOOK hhook)
{
  HarpoonMessage msg;

  msg.code = code;
  msg.wparam = wpar;
  msg.lparam = lpar;
  harpoon_post_message (hid, &msg);
  return harpoon_generic_hook_return (code, wpar, lpar, hhook);
}
  


static void
harpoon_unhook_generic(HHOOK *handle, volatile HOOKPROC *callback)
{
  if (*handle)
    {
      UnhookWindowsHookEx(*handle);
      *handle = NULL;
      *callback = NULL;
    }
}

static void
harpoon_hook_generic(HOOKPROC hf, HOOKPROC ghf, int hid, HHOOK *handle, 
		     volatile HOOKPROC *callback)
{
  *callback = hf;
  *handle = SetWindowsHookEx(hid, ghf, dll_handle, 0);
}



/**********************************************************************
 * Mouse hook
 **********************************************************************/

static LRESULT CALLBACK
harpoon_mouse_hook (int code, WPARAM wpar, LPARAM lpar)
{
  HarpoonMouseMessage msg;
  DWORD dwVersion, dwWindowsMajorVersion;
  PMOUSEHOOKSTRUCT pmhs;

  msg.message.code = code;
  msg.message.wparam = wpar;
  msg.message.lparam = 0;
  
  pmhs = (PMOUSEHOOKSTRUCT) lpar;
  msg.mouse.MOUSEHOOKSTRUCT = *pmhs;
  msg.mouse.mouseData = 0;

  dwVersion = GetVersion ();
  dwWindowsMajorVersion =  (DWORD) (LOBYTE(LOWORD(dwVersion)));
  if (dwWindowsMajorVersion >= 5)
    {
      msg.mouse.mouseData = ((PMOUSEHOOKSTRUCTEX) pmhs)->mouseData;
    }

  harpoon_post_message (WH_MOUSE, &msg.message);

  return harpoon_generic_hook_return (code, wpar, lpar, mouse_hook_handle);
}

HARPOON_API void
harpoon_unhook_mouse ()
{
  harpoon_unhook_generic (&mouse_hook_handle, &mouse_hook_callback);
}


HARPOON_API void
harpoon_hook_mouse(HOOKPROC hf)
{
  harpoon_hook_generic(hf, harpoon_mouse_hook, WH_MOUSE, 
		       &mouse_hook_handle, &mouse_hook_callback);
}



/**********************************************************************
 * Keyboard hook
 **********************************************************************/

static LRESULT CALLBACK 
harpoon_keyboard_hook (int code, WPARAM wpar, LPARAM lpar)
{
  return harpoon_generic_hook (code, wpar, lpar, WH_KEYBOARD,
                               keyboard_hook_handle);
}

HARPOON_API void
harpoon_unhook_keyboard ()
{
  harpoon_unhook_generic (&keyboard_hook_handle, &keyboard_hook_callback);
}


HARPOON_API void
harpoon_hook_keyboard (HOOKPROC hf)
{
  harpoon_hook_generic (hf, harpoon_keyboard_hook, WH_KEYBOARD, 
                        &keyboard_hook_handle, &keyboard_hook_callback);
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

