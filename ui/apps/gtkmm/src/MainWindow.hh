// MainWindow.hh --- Main info Window
//
// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include "preinclude.h"

#include <string>
#include <gtkmm/window.h>

#ifdef PLATFORM_OS_WINDOWS
#  include <windows.h>
#  include "commonui/TimerBoxControl.hh"
#endif

#include "config/IConfiguratorListener.hh"

class TimerBoxControl;
class TimerBoxGtkView;

using namespace workrave;

class MainWindow
  : public Gtk::Window
  , public workrave::config::IConfiguratorListener
{
public:
  MainWindow() = default;
  ~MainWindow() override;

  void init();
  void toggle_window();
  void open_window();
  void close_window();
  void set_can_close(bool can_close);

  void update();
  void relocate_window(int width, int height);

  sigc::signal<void> &signal_closed();
  sigc::signal<void> &signal_visibility_changed();

  bool is_visible() const;

private:
  void on_visibility_changed();
  bool on_timer_view_button_press_event(GdkEventButton *event);

private:
  //! Is the main window enabled?
  bool enabled{true};

  //! Can the user close the window?
  bool can_close{true};

  //! Controller that determines the timerbox content
  TimerBoxControl *timer_box_control{nullptr};

  //! View that displays the timerbox.
  TimerBoxGtkView *timer_box_view{nullptr};

#ifdef PLATFORM_OS_UNIX
  Gtk::Window *leader{nullptr};
#endif

  //! Location of main window.
  Gdk::Point window_location{-1, -1};

  //! Location of main window relative to current head
  Gdk::Point window_head_location{-1, -1};

  //! Relocated location of main window
  Gdk::Point window_relocated_location{-1, -1};

  //! Event triggered when the main window has been closed by the user
  sigc::signal<void> closed_signal;

  //!
  sigc::signal<void> visibility_changed_signal;

  //!
  sigc::connection visible_connection;

private:
  void setup();
  void config_changed_notify(const std::string &key) override;
  void locate_window(GdkEventConfigure *event);
  void move_to_start_position();

  // UI Events.
  bool on_delete_event(GdkEventAny *) override;
  bool on_configure_event(GdkEventConfigure *event) override;

  static void get_start_position(int &x, int &y, int &head);
  static void set_start_position(int x, int y, int head);

#ifdef PLATFORM_OS_WINDOWS
private:
  void win32_show(bool b);
  bool win32_show_retry();
  void win32_init();
  void win32_exit();

  static LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND win32_main_hwnd{0};
  HINSTANCE win32_hinstance{0};
  int show_retry_count{0};
  sigc::connection timeout_connection;
#endif
};

#endif // MAINWINDOW_HH
