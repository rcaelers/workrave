// Win32InputMonitor.cc --- ActivityMonitor for Win32
//
// Copyright (C) 2002, 2003, 2004 Raymond Penners <raymond@dotsphinx.com>
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
#include <winuser.h>
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

  harpoon_hook(on_harpoon_event);
}

//! Stops the activity monitoring.
void
Win32InputMonitor::terminate()
{
  TRACE_ENTER("Win32InputMonitor::terminate");

  harpoon_exit();

  listener = NULL;
}





void
Win32InputMonitor::on_harpoon_event(HarpoonEvent *event)
{
  switch (event->type)
    {
    case HARPOON_BUTTON_PRESS:
      listener->button_notify(0, true); // FIXME: proper parameter
      break;

    case HARPOON_BUTTON_RELEASE:
      listener->button_notify(0, false); // FIXME: proper parameter
      break;

    case HARPOON_2BUTTON_PRESS:
    case HARPOON_KEY_PRESS:
      listener->action_notify();
      break;

    case HARPOON_MOUSE_WHEEL:
      listener->mouse_notify(event->mouse.x, event->mouse.y,
                             event->mouse.wheel);
      break;

    case HARPOON_KEY_RELEASE:
      listener->keyboard_notify(0, 0);
      break;

    case HARPOON_MOUSE_MOVE:
      listener->mouse_notify(event->mouse.x, event->mouse.y, 0);
      break;
    }
}
