// Applet.h --- Applet 
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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
// TODO: release CORBA memory.

#ifndef APPLET_H
#define APPLET_H

#include "CoreInterface.hh"

#define APPLET_WINDOW_CLASS_NAME "WorkraveApplet"
#define APPLET_BAR_TEXT_MAX_LENGTH 16

#define APPLET_MESSAGE_HEARTBEAT 0
#define APPLET_MESSAGE_MENU 1

struct AppletHeartbeatData
{
  bool enabled;
  short slots[BREAK_ID_SIZEOF];

  char bar_text[BREAK_ID_SIZEOF][APPLET_BAR_TEXT_MAX_LENGTH];

  short bar_secondary_color[BREAK_ID_SIZEOF];
  int bar_secondary_val[BREAK_ID_SIZEOF];
  int bar_secondary_max[BREAK_ID_SIZEOF];

  short bar_primary_color[BREAK_ID_SIZEOF];
  int bar_primary_val[BREAK_ID_SIZEOF];
  int bar_primary_max[BREAK_ID_SIZEOF];
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
  HWND command_window;
  AppletMenuItemData items[APPLET_MAX_MENU_ITEMS];
};


#endif /* APPLET_H */
