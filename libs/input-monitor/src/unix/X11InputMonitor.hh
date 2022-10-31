// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef X11INPUTMONITOR_HH
#define X11INPUTMONITOR_HH

#include <string>

#include <thread>
#include <chrono>

#include <X11/X.h>
#include <X11/Xlib.h>

#include "InputMonitor.hh"

//! Activity monitor for a local X server.
class X11InputMonitor : public InputMonitor
{
public:
  //! Constructor.
  X11InputMonitor(const char *display_name);

  //! Destructor.
  ~X11InputMonitor() override;

  //! Initialize
  bool init() override;

  //! Terminate the monitor.
  void terminate() override;

private:
  //! The monitor's execution thread.
  void run();

  void error_trap_enter();
  void error_trap_exit();

private:
  //! Internal X magic
  void set_event_mask(Window window);

  //! Internal X magic
  void set_all_events(Window window);

  //! Handle a key press event.
  void handle_keypress(XEvent *event);

  //! Handle a window creation event.
  void handle_create(XEvent *event);

  //! Handle a mouse button event.
  void handle_button(XEvent *event);

private:
  //! The X11 display name.
  const char *x11_display_name;

  //! The X11 display handle.
  Display *x11_display;

  //! The X11 root window handle.
  Window root_window;

  //! Abort the main loop
  bool abort;

  //! The activity monitor thread.
  std::shared_ptr<std::thread> monitor_thread;
};

#endif // X11INPUTMONITOR_HH
