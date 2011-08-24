// AppletWindow.hh --- Applet window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011 Rob Caelers & Raymond Penners
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

#ifndef W32APPLETWINDOW_HH
#define W32APPLETWINDOW_HH

#include "preinclude.h"
#include <windows.h>
#include <string>
#include <gdk/gdkwin32.h>

#include "TimerBoxViewBase.hh"
#include "ITimeBar.hh"
#include "Applet.hh"
#include "AppletWindow.hh"

class W32AppletWindow : public IAppletWindow, public TimerBoxViewBase
{
public:
  W32AppletWindow();
  virtual ~W32AppletWindow();

  virtual AppletState activate_applet();
  virtual void deactivate_applet();

  void set_slot(BreakId  id, int slot);
  void set_time_bar(BreakId id,
                    std::string text,
                    ITimeBar::ColorId primary_color,
                    int primary_value, int primary_max,
                    ITimeBar::ColorId secondary_color,
                    int secondary_value, int secondary_max);
  void update_view();
  void update_time_bars();
  void update_menu();
  void set_enabled(bool enabled);
  void set_geometry(Orientation orientation, int size);

  void init_menu(HWND dest);
  void add_menu(const char *text, short cmd, int flags);

  GdkFilterReturn win32_filter_func (void *xevent, GdkEvent *event);
  bool on_applet_command(int command);

  enum MenuFlag
  {
    MENU_FLAG_TOGGLE = APPLET_MENU_FLAG_TOGGLE,
    MENU_FLAG_SELECTED = APPLET_MENU_FLAG_SELECTED,
    MENU_FLAG_POPUP = APPLET_MENU_FLAG_POPUP
  };

private:
  HWND get_applet_window();

  static DWORD WINAPI run_event_pipe_static(LPVOID);

private:
  void run_event_pipe();

  HWND applet_window;
  bool menu_sent;

  bool local_menu_ready;
  AppletHeartbeatData local_heartbeat_data;
  AppletMenuData local_menu_data;
  HWND local_applet_window;

  AppletHeartbeatData heartbeat_data;
  AppletMenuData menu_data;
  CRITICAL_SECTION heartbeat_data_lock;
  HANDLE heartbeat_data_event;
  HANDLE thread_handle;
  volatile DWORD thread_id;
};

#endif // W32APPLETWINDOW_HH
