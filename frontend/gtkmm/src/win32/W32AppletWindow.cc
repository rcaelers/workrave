// AppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "W32AppletWindow.hh"
#include "TimerBoxControl.hh"
#include "Applet.hh"
#include "GUI.hh"
#include "Menus.hh"

W32AppletWindow::W32AppletWindow()
{
  TRACE_ENTER("W32AppletWindow::W32AppletWindow");

  timer_box_view = this;
  applet_window = NULL;
  heartbeat_data.enabled = false;
  init_menu(NULL);

  // Intentionally last line, as this one calls W32AW::set_enabled(), e.g.
  timer_box_control = new TimerBoxControl("applet", *this);

  TRACE_EXIT();
}

W32AppletWindow::~W32AppletWindow()
{
  TRACE_ENTER("W32AppletWindow::~W32AppletWindow");
  delete timer_box_control;
  TRACE_EXIT();
}


AppletWindow::AppletMode
W32AppletWindow::get_applet_mode() const
{
  return APPLET_W32;
}





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



void
W32AppletWindow::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_slot", int(id) << ", " << slot);
  heartbeat_data.slots[slot] = (short) id;
  TRACE_EXIT();
}

void
W32AppletWindow::set_time_bar(BreakId id,
                                 std::string text,
                                 ITimeBar::ColorId primary_color,
                                 int primary_val, int primary_max,
                                 ITimeBar::ColorId secondary_color,
                                 int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_time_bar", int(id) << "=" << text);
  strncpy(heartbeat_data.bar_text[id], text.c_str(), APPLET_BAR_TEXT_MAX_LENGTH-1);
  heartbeat_data.bar_text[id][APPLET_BAR_TEXT_MAX_LENGTH-1] = '\0';
  heartbeat_data.bar_primary_color[id] = primary_color;
  heartbeat_data.bar_primary_val[id] = primary_val;
  heartbeat_data.bar_primary_max[id] = primary_max;
  heartbeat_data.bar_secondary_color[id] = secondary_color;
  heartbeat_data.bar_secondary_val[id] = secondary_val;
  heartbeat_data.bar_secondary_max[id] = secondary_max;
  TRACE_EXIT();
}

void
W32AppletWindow::set_tip(std::string tip)
{
  (void) tip;
}

void
W32AppletWindow::set_icon(IconType type)
{
  (void) type;
}


void
W32AppletWindow::update_view()
{
  TRACE_ENTER("W32AppletWindow::update_view");
  update_time_bars();
  update_menu();
  TRACE_EXIT();
}

void
W32AppletWindow::update_menu()
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
W32AppletWindow::update_time_bars()
{
  TRACE_ENTER("W32AppletWindow::update_time_bars");
  HWND hwnd = get_applet_window();
  if (hwnd != NULL)
    {
      COPYDATASTRUCT msg;
      msg.dwData = APPLET_MESSAGE_HEARTBEAT;
      msg.cbData = sizeof(AppletHeartbeatData);
      msg.lpData = &heartbeat_data;
      TRACE_MSG("sending: enabled=" << heartbeat_data.enabled);
      for (size_t i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          TRACE_MSG("sending: slots[]=" << heartbeat_data.slots[i]);
        }
      SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM) &msg);
    }
  TRACE_EXIT();
}

HWND
W32AppletWindow::get_applet_window()
{
  TRACE_ENTER("W32AppletWindow::get_applet_window");
  if (applet_window == NULL || !IsWindow(applet_window))
    {
      HWND taskbar = FindWindow("Shell_TrayWnd",NULL);
      applet_window = RecursiveFindWindow(taskbar, APPLET_WINDOW_CLASS_NAME);
      menu_sent = false;
    }
  TRACE_RETURN((applet_window ? "Applet found" : "Applet not found"));
  return applet_window;
}

void
W32AppletWindow::set_enabled(bool enabled)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_enabled", enabled);
  heartbeat_data.enabled = enabled;
  TRACE_EXIT();
}


void
W32AppletWindow::init_menu(HWND hwnd)
{
  menu_data.num_items = 0;
  menu_sent = false;
  menu_data.command_window = hwnd;
}

void
W32AppletWindow::add_menu(const char *text, short cmd, int flags)
{
  AppletMenuItemData *d = &menu_data.items[menu_data.num_items++];
  d->command = cmd;
  strcpy(d->text, text);
  d->flags = flags;
}


AppletWindow::AppletState
W32AppletWindow::activate_applet()
{
  return APPLET_STATE_VISIBLE;
}

void
W32AppletWindow::deactivate_applet()
{
}

void
W32AppletWindow::set_geometry(Orientation orientation, int size)
{
  (void) orientation;
  (void) size;
}


bool
W32AppletWindow::on_applet_command(int command)
{
  Menus *menus = Menus::get_instance();
  menus->on_applet_command(command);

  return false;
}


GdkFilterReturn
W32AppletWindow::win32_filter_func (void     *xevent,
                                    GdkEvent *event)
{
  (void) event;
  MSG *msg = (MSG *) xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;
  switch (msg->message)
    {
    case WM_USER:
      {
        sigc::slot<bool> my_slot = sigc::bind(sigc::mem_fun(*this, &W32AppletWindow::on_applet_command),
                                              (int) msg->wParam);
        Glib::signal_idle().connect(my_slot);

        ret = GDK_FILTER_REMOVE;
      }
      break;

    case WM_USER + 1:
      {
        timer_box_control->force_cycle();
        ret = GDK_FILTER_REMOVE;
      }
      break;
    }
  return ret;
}
