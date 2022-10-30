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

#ifndef RECORDINPUTMONITOR_HH
#define RECORDINPUTMONITOR_HH

#include <thread>
#include <string>
#include <chrono>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <X11/extensions/record.h>

#include "InputMonitor.hh"

//! Activity monitor for a local X server.
class RecordInputMonitor : public InputMonitor
{
public:
  //! Constructor.
  RecordInputMonitor(const char *display_name);

  //! Destructor.
  ~RecordInputMonitor() override;

  //! Initialize
  bool init() override;

  //! Terminate the monitor.
  void terminate() override;

private:
  //! The monitor's execution thread.
  void run();

  void error_trap_enter();
  void error_trap_exit();

  //! Initialize
  bool init_xrecord();

  //! Stop the capturing.
  bool stop_xrecord();

  void handle_xrecord_key_event(XRecordInterceptData *data);
  void handle_xrecord_motion_event(XRecordInterceptData *data);
  void handle_xrecord_button_event(XRecordInterceptData *data);
  void handle_xrecord_device_key_event(bool press, XRecordInterceptData *data);
  void handle_xrecord_device_motion_event(XRecordInterceptData *data);
  void handle_xrecord_device_button_event(XRecordInterceptData *data);

  static void handle_xrecord_callback(XPointer closure, XRecordInterceptData *data);

private:
  //! The X11 display name.
  const char *x11_display_name;

  //! The X11 display handle.
  Display *x11_display;

  //! Abort the main loop
  bool abort;

  //! The activity monitor thread.
  std::shared_ptr<std::thread> monitor_thread;

  //! XRecord context. Defines clients and events to capture.
  XRecordContext xrecord_context;

  //! X Connection for event capturing.
  Display *xrecord_datalink;

  // Event base for Xinput events
  static int xi_event_base;
};

#endif // RECORDINPUTMONITOR_HH
