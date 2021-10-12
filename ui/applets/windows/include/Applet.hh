// Copyright (C) 2001 - 2007 Rob Caelers & Raymond Penners
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

#ifndef APPLET_H
#define APPLET_H

#include "core/CoreTypes.hh"

#define APPLET_WINDOW_CLASS_NAME "WorkraveApplet"
#define APPLET_BAR_TEXT_MAX_LENGTH 16

#define APPLET_MESSAGE_HEARTBEAT 0
#define APPLET_MESSAGE_MENU 1

/*!!!!!!!!!!!!!!!!!!!
On Windows x64, the installed Applet will be 64-bit.

Therefore Workrave (32-bit) could be passing structures to a
64-bit applet. And so we must ensure that all members of any
passed structure have types that are the same size on both
32 and 64 bit Windows.

All structures declared in this file are used by both
Workrave (x86) and the applet (x86 & x64).
*/

struct AppletHeartbeatData
{
  volatile bool enabled;
  short slots[workrave::BREAK_ID_SIZEOF];

  char bar_text[workrave::BREAK_ID_SIZEOF][APPLET_BAR_TEXT_MAX_LENGTH];

  short bar_secondary_color[workrave::BREAK_ID_SIZEOF];
  int bar_secondary_val[workrave::BREAK_ID_SIZEOF];
  int bar_secondary_max[workrave::BREAK_ID_SIZEOF];

  short bar_primary_color[workrave::BREAK_ID_SIZEOF];
  int bar_primary_val[workrave::BREAK_ID_SIZEOF];
  int bar_primary_max[workrave::BREAK_ID_SIZEOF];
};

#define APPLET_MAX_MENU_ITEMS 16
#define APPLET_MENU_TEXT_MAX_LENGTH 48

#define APPLET_MENU_FLAG_TOGGLE 1
#define APPLET_MENU_FLAG_SELECTED 2
#define APPLET_MENU_FLAG_POPUP 4

struct AppletMenuItemData
{
  char text[APPLET_MENU_TEXT_MAX_LENGTH]; // mbs
  int flags;
  short command;
};

struct AppletMenuData
{
  short num_items;

  /*
  MSDN notes:
  Handles have 32 significant bits on 64-bit Windows
  .
  We must ensure that our types are the same size
  on both 64 and 32 bit systems (see x64 comment).

  We will pass the command_window HWND as a LONG:
  */
  LONG command_window;

  AppletMenuItemData items[APPLET_MAX_MENU_ITEMS];
};

#endif /* APPLET_H */
