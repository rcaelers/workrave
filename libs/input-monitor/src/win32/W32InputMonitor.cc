// W32InputMonitor.cc --- ActivityMonitor for W32
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2010 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <assert.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>

#include <windows.h>
#include <winuser.h>
#include "debug.hh"
#include "W32InputMonitor.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "timeutil.h"
#include "Harpoon.hh"

#ifndef HAVE_STRUCT_MOUSEHOOKSTRUCT
typedef struct tagMOUSEHOOKSTRUCT
{
  POINT pt;
  HWND hwnd;
  UINT wHitTestCode;
  DWORD dwExtraInfo;
} MOUSEHOOKSTRUCT, FAR *LPMOUSEHOOKSTRUCT, *PMOUSEHOOKSTRUCT;
#endif

#ifndef HAVE_STRUCT_MOUSEHOOKSTRUCTEX
typedef struct
{
  struct tagMOUSEHOOKSTRUCT MOUSEHOOKSTRUCT;
  DWORD mouseData;
} MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;
#endif

W32InputMonitor *W32InputMonitor::singleton = NULL;

W32InputMonitor::W32InputMonitor() {}

W32InputMonitor::~W32InputMonitor()
{
  terminate();
}

bool
W32InputMonitor::init()
{
  if (singleton == NULL)
    {
      singleton = this;
      return Harpoon::init(on_harpoon_event);
    }
  else
    {
      return false;
    }
}

//! Stops the activity monitoring.
void
W32InputMonitor::terminate()
{
  Harpoon::terminate();
}

void
W32InputMonitor::on_harpoon_event(HarpoonEvent *event)
{
  switch (event->type)
    {
    case HARPOON_BUTTON_PRESS:
      singleton->fire_button(true);
      break;

    case HARPOON_BUTTON_RELEASE:
      singleton->fire_button(false);
      break;

    case HARPOON_2BUTTON_PRESS:
    case HARPOON_KEY_PRESS:
      singleton->fire_action();
      break;

    case HARPOON_MOUSE_WHEEL:
      singleton->fire_mouse(event->mouse.x, event->mouse.y, event->mouse.wheel);
      break;

    case HARPOON_KEY_RELEASE:
      singleton->fire_keyboard(false);
      break;

    case HARPOON_MOUSE_MOVE:
      singleton->fire_mouse(event->mouse.x, event->mouse.y, 0);
      break;

    default:
      break;
    }
}
