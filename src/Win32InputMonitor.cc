// Win32InputMonitor.cc --- ActivityMonitor for Win32
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#include <assert.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include "debug.hh"
#include "Win32InputMonitor.hh"
#include "InputMonitorListenerInterface.hh"
#include "timeutil.h"
#include "harpoon.h"

#ifndef HAVE_STRUCT_MOUSEHOOKSTRUCT
typedef struct tagMOUSEHOOKSTRUCT {
    POINT   pt;
    HWND    hwnd;
    UINT    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT, FAR *LPMOUSEHOOKSTRUCT, *PMOUSEHOOKSTRUCT;
#endif

typedef struct {
    struct tagMOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
    DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;


InputMonitorListenerInterface *Win32InputMonitor::listener = NULL;

Win32InputMonitor::Win32InputMonitor()
{
}


Win32InputMonitor::~Win32InputMonitor()
{
}

void
Win32InputMonitor::init(InputMonitorListenerInterface *l)
{
  TRACE_ENTER("Win32InputMonitor::init");

  assert(! listener);
  listener = l;

  // FIXME: should InputMonitorListenerInterface::init() have a
  // return value?
  BOOL b = harpoon_init();
  assert(b);

  harpoon_hook_mouse(mouse_hook);
  harpoon_hook_keyboard(keyboard_hook);
}

//! Stops the activity monitoring.
void
Win32InputMonitor::terminate()
{
  TRACE_ENTER("Win32InputMonitor::terminate");

  harpoon_unhook_mouse();
  harpoon_unhook_keyboard();

  harpoon_exit();

  listener = NULL;
}


LRESULT CALLBACK
Win32InputMonitor::keyboard_hook(int code, WPARAM wparam, LPARAM lparam)
{
  TRACE_ENTER("Win32InputMonitor::keyboard_hook");
  bool pressed = (lparam & (1 << 31)) == 0;
  bool prevpressed = (lparam & (1 << 30)) != 0;
  if (pressed && !prevpressed && code == HC_ACTION)
    {
      // FIXME: supply decent parameters to keyboard_notify()
      listener->keyboard_notify(0, 0);
    }
  else
    {
      listener->action_notify();
    }
}

LRESULT CALLBACK
Win32InputMonitor::mouse_hook(int code, WPARAM wparam, LPARAM lparam)
{
  PMOUSEHOOKSTRUCTEX mhs = (PMOUSEHOOKSTRUCTEX) lparam;
  //    TRACE_ENTER_MSG("Win32InputMonitor::mouse_hook",
  //                    wparam << ", (" << mx
  //                    << ", " << my
  //                    << ", " << mw << ")");
  if (code == HC_ACTION)
    {
      switch (wparam)
        {
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
          listener->button_notify(0, true); // FIXME: proper parameter
          break;
  
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
          listener->button_notify(0, false); // FIXME: proper parameter
          break;
        
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
          listener->action_notify();
          break;
        
        default:
          int mx = mhs->MOUSEHOOKSTRUCT.pt.x;
          int my = mhs->MOUSEHOOKSTRUCT.pt.y;
          int mw = wparam == WM_MOUSEWHEEL ? HIWORD(mhs->mouseData) : 0;
          listener->mouse_notify(mx, my, mw);
        }
    }
}

