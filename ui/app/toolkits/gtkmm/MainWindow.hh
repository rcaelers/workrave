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

#include <string>
#include <gtkmm.h>

#include "utils/Signals.hh"

#include "ui/IApplicationContext.hh"
#include "ui/IToolkit.hh"
#include "commonui/MenuModel.hh"
#include "ToolkitMenu.hh"

class TimerBoxControl;
class TimerBoxGtkView;

class MainWindow
  : public Gtk::ApplicationWindow
  , public workrave::utils::Trackable
{
public:
  using closed_signal_t = sigc::signal<void()>;

  MainWindow(std::shared_ptr<IApplicationContext> app);
  ~MainWindow() override;

  void init();
  void toggle_window();
  void open_window();
  void close_window();
  void set_can_close(bool can_close);

  void update();
  void relocate_window(int width, int height);

  closed_signal_t &signal_closed();

private:
  bool on_timer_view_button_press_event(const GdkEventButton *event);

  int map_to_head(int &x, int &y);
  void map_from_head(int &x, int &y, int head);
  bool bound_head(int &x, int &y, int width, int height, int &head);

private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<IToolkit> toolkit;

  //! Is the main window enabled?
  bool enabled{false};

  //! Can the user close the window?
  bool can_close{false};

  //! Controller that determines the timerbox content
  TimerBoxControl *timer_box_control{nullptr};

  //! View that displays the timerbox.
  TimerBoxGtkView *timer_box_view{nullptr};

  std::shared_ptr<ToolkitMenu> menu;

#if defined(PLATFORM_OS_UNIX)
  Gtk::Window *leader{nullptr};
#endif

  //! Location of main window.
  Gdk::Point window_location{-1, -1};

  //! Location of main window relative to current head
  Gdk::Point window_head_location{-1, -1};

  //! Relocated location of main window
  Gdk::Point window_relocated_location{-1, -1};

  //! Event triggered when the main window has been closed by the user
  closed_signal_t closed_signal;

private:
  void setup();
  void locate_window(GdkEventConfigure *event);
  void move_to_start_position();

  // UI Events.
  bool on_delete_event(GdkEventAny *) override;
  bool on_configure_event(GdkEventConfigure *event) override;

  static void get_start_position(int &x, int &y, int &head);
  static void set_start_position(int x, int y, int head);
};

#endif // MAINWINDOW_HH
