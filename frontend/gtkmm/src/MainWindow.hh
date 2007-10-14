// MainWindow.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "preinclude.h"

#include <string>

class GUI;
class TimeBar;
class NetworkLogDialog;
class TimerBoxControl;
class TimerBoxGtkView;

#include <gtkmm/window.h>

#ifdef WIN32
#include <windows.h>
#include "TimerBoxControl.hh"
#endif

#include "IConfiguratorListener.hh"

namespace Gtk
{
  class Menu;
}

using namespace workrave;

class MainWindow :
  public Gtk::Window,
  public IConfiguratorListener
{
public:
  MainWindow();
  ~MainWindow();

  void open_window();
  void close_window();
  void toggle_window();
  void set_applet_active(bool a);
  //bool get_iconified() const;

  void update();
  void relocate_window(int width, int height);

  void on_activate();

  static bool get_always_on_top();
  static void set_always_on_top(bool b);

protected:
  bool on_button_press_event(GdkEventButton *event);

private:
  //! Window enabled
  bool enabled;

  //! Connection to the delete_event signal.
  // SigC::Connection delete_connection;

  //! Connection to the timeout timer.
  // SigC::Connection timer_connection;

  //! Table containing all timer information
  TimerBoxControl *timer_box_control;

  //! Table containing all timer information
  TimerBoxGtkView *timer_box_view;

  //! Is the monitoring function suspended?
  bool monitor_suspended;

  //! Is the visible?
  bool visible;

  //! Applet active?
  bool applet_active;

#ifdef HAVE_X
  Gtk::Window *leader;
#endif

  //! Location of main window.
  Gdk::Point window_location;

  //! Location of main window relative to current head
  Gdk::Point window_head_location;

  //! Relocated location of main window
  Gdk::Point window_relocated_location;

private:
  //
  void init();
  void setup();
  void config_changed_notify(const std::string &key);
  void locate_window(GdkEventConfigure *event);
  void move_to_start_position();

  // Events.
  bool on_delete_event(GdkEventAny*);
  bool on_window_state_event(GdkEventWindowState *event);
  bool on_configure_event(GdkEventConfigure *event);

public:
  static void set_start_in_tray(bool b);
  static bool get_start_in_tray();

  static void get_start_position(int &x, int &y, int &head);
  static void set_start_position(int x, int y, int head);

  static const std::string CFG_KEY_MAIN_WINDOW;
  static const std::string CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP;
  static const std::string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const std::string CFG_KEY_MAIN_WINDOW_X;
  static const std::string CFG_KEY_MAIN_WINDOW_Y;
  static const std::string CFG_KEY_MAIN_WINDOW_HEAD;

#ifdef WIN32
private:
  void win32_show(bool b);
  void win32_init();
  void win32_exit();

  static LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam);

  HWND win32_main_hwnd;
  HINSTANCE win32_hinstance;

  SigC::Connection timeout_connection;
#endif
};


// inline bool
// MainWindow::get_iconified() const
// {
//   return iconified;
// }

#endif // MAINWINDOW_HH
