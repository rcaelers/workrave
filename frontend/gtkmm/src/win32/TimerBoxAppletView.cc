// TimerBoxGtkView.cc --- Timers Widgets
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TimerBoxAppletView.hh"
#include "Applet.hh"

static HWND
RecursiveFindWindow(HWND hwnd, LPCTSTR lpClassName)
{
  static char buf[80];
  int num = GetClassName(hwnd, buf, sizeof(buf)-1);
  buf[num] = 0;
  HWND ret = NULL;
  
  if (! stricmp(lpClassName, buf))
    {
      ret =  hwnd;
    }
  else
    {
      HWND child = FindWindowEx(hwnd, 0, NULL, NULL);
      while (child != NULL)
        {
          ret = RecursiveFindWindow(child, lpClassName);
          if (ret)
            {
              break;
            }
          child = FindWindowEx(hwnd, child, NULL, NULL);
        }
    }
  return ret;
}
                                
//! Constructor.
TimerBoxAppletView::TimerBoxAppletView()
{
  applet_window = NULL;
  heartbeat_data.enabled = false;
  init_menu(NULL);
}
  


//! Destructor.
TimerBoxAppletView::~TimerBoxAppletView()
{
}



void
TimerBoxAppletView::set_slot(BreakId id, int slot)
{
  heartbeat_data.slots[slot] = (short) id;
}

void
TimerBoxAppletView::set_time_bar(BreakId id,
                                 std::string text,
                                 TimeBarInterface::ColorId primary_color,
                                 int primary_val, int primary_max,
                                 TimeBarInterface::ColorId secondary_color,
                                 int secondary_val, int secondary_max)
{
  strncpy(heartbeat_data.bar_text[id], text.c_str(), APPLET_BAR_TEXT_MAX_LENGTH-1);
  heartbeat_data.bar_text[id][APPLET_BAR_TEXT_MAX_LENGTH-1] = '\0';
  heartbeat_data.bar_primary_color[id] = primary_color;
  heartbeat_data.bar_primary_val[id] = primary_val;
  heartbeat_data.bar_primary_max[id] = primary_max;
  heartbeat_data.bar_secondary_color[id] = secondary_color;
  heartbeat_data.bar_secondary_val[id] = secondary_val;
  heartbeat_data.bar_secondary_max[id] = secondary_max;
}

void
TimerBoxAppletView::set_tip(string tip)
{
}


void
TimerBoxAppletView::update()
{
  update_time_bars();
  update_menu();
}

void
TimerBoxAppletView::update_menu()
{
  if (menu_sent)
    return;

  HWND hwnd = get_applet_window();
  if (hwnd != NULL)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_MENU;
      msg.cbData = sizeof(AppletMenuData);
      msg.lpData = &menu_data;
      SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM) &msg);

      menu_sent = true;
    }
}

void
TimerBoxAppletView::update_time_bars()
{
  HWND hwnd = get_applet_window();
  if (hwnd != NULL)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_HEARTBEAT;
      msg.cbData = sizeof(AppletHeartbeatData);
      msg.lpData = &heartbeat_data;
      SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM) &msg);
    }
}
  
HWND
TimerBoxAppletView::get_applet_window()
{
  if (applet_window == NULL || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindow("Shell_TrayWnd",NULL);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }
  return applet_window;
}

void
TimerBoxAppletView::set_enabled(bool enabled)
{
  heartbeat_data.enabled = enabled;
}


void
TimerBoxAppletView::init_menu(HWND hwnd)
{
  menu_data.num_items = 0;
  menu_sent = false;
  menu_data.command_window = hwnd;
}

void
TimerBoxAppletView::add_menu(const char *text, short cmd, int flags)
{
  AppletMenuItemData *d = &menu_data.items[menu_data.num_items++];
  d->command = cmd;
  strcpy(d->text, text);
  d->flags = flags;
}
