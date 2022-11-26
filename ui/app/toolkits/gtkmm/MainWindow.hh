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

  explicit MainWindow(std::shared_ptr<IApplicationContext> app);
  ~MainWindow() override;

  void set_can_close(bool can_close);

  void open_window();
  void close_window();
  void update();

  closed_signal_t &signal_closed();

private:
  void init();

  int convert_display_to_monitor(int &x, int &y);
  void convert_monitor_to_display(int &x, int &y, int head);
  void locate_window(GdkEventConfigure *event);
  void move_to_start_position();

  bool on_timer_view_button_press_event(const GdkEventButton *event);
  void on_enabled_changed();

  // UI Events.
  bool on_delete_event(GdkEventAny *) override;

private:
  std::shared_ptr<IApplicationContext> app;

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

  //! Event triggered when the main window has been closed by the user
  closed_signal_t closed_signal;
};

#endif // MAINWINDOW_HH
