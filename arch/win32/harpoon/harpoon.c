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
#include "harpoon.h"

#define HARPOON_MAILSLOT_NAME "\\\\.\\mailslot\\harpoon"

#pragma comment(linker, "/SECTION:.shared,RWS")
#pragma data_seg(".shared")
HHOOK mouse_hook_handle = NULL;
HHOOK keyboard_hook_handle = NULL;
BOOL block_input = FALSE;
HWND unblocked_window = NULL;
#pragma data_seg()

static HANDLE dll_handle = NULL;
static volatile HOOKPROC mouse_hook_callback = NULL;
static volatile HOOKPROC keyboard_hook_callback = NULL;
static HANDLE thread_handle = NULL;
static HANDLE mailslot;


struct harpoon_mailslot_message
{
  int hook;
  int code;
  WPARAM wparam;
  LPARAM lparam;
};

typedef struct {
  MOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
  DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;



DWORD WINAPI 
harpoon_thread_proc(LPVOID lpParameter)
{
  BOOL running = TRUE;
  while (running)
    {
      struct harpoon_mailslot_message msg;
      DWORD read;
      
      ReadFile(mailslot, &msg, sizeof(msg), &read, NULL);


      switch (msg.hook)
	{
	case WH_KEYBOARD:
          {
            HOOKPROC proc;
            proc = keyboard_hook_callback;
            if (proc != NULL)
              {
                (*proc)(msg.code, msg.wparam, msg.lparam);
              }
	  }
          break;
	case WH_MOUSE:
	  {
	    MOUSEHOOKSTRUCTEX mhs;
            HOOKPROC proc;
            
	    ReadFile(mailslot, &mhs, sizeof(mhs), &read, NULL);
	    msg.lparam = (LPARAM) &mhs;
            proc = mouse_hook_callback;
            if (proc != NULL)
              {
                (*proc)(msg.code, msg.wparam, msg.lparam);
              }
	    break;
	  }
	case ~0:
          running = FALSE;
          break;
	}
    }
  return 0;
}


void
harpoon_mailslot_send_data(void *buf, int buf_len)
{
  HANDLE h = CreateFile(HARPOON_MAILSLOT_NAME, GENERIC_WRITE, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING,  
			FILE_ATTRIBUTE_NORMAL, NULL);
  if (h != INVALID_HANDLE_VALUE)
    {
      DWORD written;

      WriteFile(h, buf, buf_len, &written, NULL);
      CloseHandle(h);
    }
}


void
harpoon_mailslot_send_message(int hook, int code, WPARAM wparam, LPARAM lparam)
{
  struct harpoon_mailslot_message msg;

  msg.hook = hook;
  msg.code = code;
  msg.wparam = wparam;
  msg.lparam = lparam;
  harpoon_mailslot_send_data(&msg, sizeof(msg));
}



/**********************************************************************
 * Initialisation
 **********************************************************************/

HARPOON_API BOOL
harpoon_init(void)
{
  SECURITY_ATTRIBUTES sa;
  BOOL rc;

  block_input = FALSE;

  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;
  sa.lpSecurityDescriptor = NULL;

  mailslot = CreateMailslot(HARPOON_MAILSLOT_NAME, 0, MAILSLOT_WAIT_FOREVER,
			    &sa);
  rc = (mailslot != INVALID_HANDLE_VALUE);
  if (rc)
    {
      DWORD id;
      thread_handle = CreateThread(&sa, 0, harpoon_thread_proc, NULL, 0, &id);
      rc = (thread_handle != NULL);
    }

  if (! rc)
    {
      harpoon_exit();
    }
  return rc;
}

HARPOON_API void
harpoon_exit(void)
{
  if (mailslot != INVALID_HANDLE_VALUE)
    {
      if (thread_handle)
        {
	  harpoon_mailslot_send_message(~0, 0, 0, 0);
	  WaitForSingleObject(thread_handle, INFINITE);
	  thread_handle = NULL;
        }
      CloseHandle(mailslot);
      mailslot = NULL;
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
      HWND target_window = NULL;
      blocked = TRUE;
      if (hhook == mouse_hook_handle)
        {
          PMOUSEHOOKSTRUCT pmhs = (PMOUSEHOOKSTRUCT) lpar;
          target_window = pmhs->hwnd;
        }
      if (target_window != NULL && unblocked_window != NULL)
        {
          blocked = target_window != unblocked_window &&
            GetParent(target_window) != unblocked_window;
        }
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
  harpoon_mailslot_send_message(hid, code, wpar, lpar);
  return harpoon_generic_hook_return(code, wpar, lpar, hhook);
}
  


HARPOON_API void
harpoon_unhook_generic(HHOOK *handle, volatile HOOKPROC *callback)
{
  if (*handle)
    {
      UnhookWindowsHookEx(*handle);
      *handle = NULL;
      *callback = NULL;
    }
}

HARPOON_API void
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
harpoon_mouse_hook(int code, WPARAM wpar, LPARAM lpar)
{
  DWORD dwVersion, dwWindowsMajorVersion;
  MOUSEHOOKSTRUCTEX mhsex;
  PMOUSEHOOKSTRUCT pmhs;

  harpoon_mailslot_send_message(WH_MOUSE, code, wpar, 0L);

  pmhs = (PMOUSEHOOKSTRUCT) lpar;
  mhsex.MOUSEHOOKSTRUCT = *pmhs;
  mhsex.mouseData = 0;

  dwVersion = GetVersion();
  dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
  if (dwWindowsMajorVersion >= 5)
    {
      mhsex.mouseData = ((PMOUSEHOOKSTRUCTEX) pmhs)->mouseData;
    }

  harpoon_mailslot_send_data(&mhsex, sizeof(mhsex));

  return harpoon_generic_hook_return(code, wpar, lpar, mouse_hook_handle);
}

HARPOON_API void
harpoon_unhook_mouse()
{
  harpoon_unhook_generic(&mouse_hook_handle, &mouse_hook_callback);
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
harpoon_keyboard_hook(int code, WPARAM wpar, LPARAM lpar)
{
  return harpoon_generic_hook(code, wpar, lpar, WH_KEYBOARD, keyboard_hook_handle);
}

HARPOON_API void
harpoon_unhook_keyboard()
{
  harpoon_unhook_generic(&keyboard_hook_handle, &keyboard_hook_callback);
}


HARPOON_API void
harpoon_hook_keyboard(HOOKPROC hf)
{
  harpoon_hook_generic(hf, harpoon_keyboard_hook, WH_KEYBOARD, 
		       &keyboard_hook_handle, &keyboard_hook_callback);
}




/**********************************************************************
 * Misc
 **********************************************************************/

HARPOON_API void
harpoon_block_input(BOOL block, HWND unblocked)
{
  block_input = block;
  unblocked_window = unblocked;
}

BOOL APIENTRY 
DllMain( HANDLE hModule, 
	 DWORD  ul_reason_for_call, 
	 LPVOID lpReserved
	 )
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

