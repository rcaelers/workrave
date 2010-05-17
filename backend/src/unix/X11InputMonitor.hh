// X11InputMonitor.hh --- ActivityMonitor for X11
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010 Rob Caelers <robc@krandor.nl>
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

#include <X11/X.h>
#include <X11/Xlib.h>

#ifdef HAVE_XRECORD
#include <X11/extensions/record.h>
#endif

#include "InputMonitor.hh"

#include "Runnable.hh"
#include "Thread.hh"

//! Activity monitor for a local X server.
class X11InputMonitor :
  public InputMonitor,
  public Runnable
{
public:
  //! Constructor.
  X11InputMonitor(const std::string &display_name);

  //! Destructor.
  virtual ~X11InputMonitor();

  //! Initialize
  virtual bool init();

  //! Terminate the monitor.
  virtual void terminate();

private:

  //! The monitor's execution thread.
  virtual void run();

  //! the events execution thread.
  void run_events();

  void error_trap_enter();
  void error_trap_exit();
  
#ifdef HAVE_XRECORD
  //! Initialize
  bool init_xrecord();

  //! the XRecord execution thread.
  void run_xrecord();

  //! Stop the capturing.
  bool stop_xrecord();
#endif

#ifdef HAVE_XRECORD
  void handle_xrecord_handle_key_event(XRecordInterceptData *data);
  void handle_xrecord_handle_motion_event(XRecordInterceptData *data);
  void handle_xrecord_handle_button_event(XRecordInterceptData *data);
  void handle_xrecord_handle_device_key_event(XRecordInterceptData *data);
  void handle_xrecord_handle_device_motion_event(XRecordInterceptData *data);
  void handle_xrecord_handle_device_button_event(XRecordInterceptData *data);

  static void handle_xrecord_callback(XPointer closure, XRecordInterceptData * data);
#endif

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
  std::string x11_display_name;

  //! The X11 display handle.
  Display *x11_display;

  //! The X11 root window handle.
  Window root_window;

  //! The activity monitor thread.
  Thread *monitor_thread;

  //! Abort the main loop
  bool abort;

#ifdef HAVE_XRECORD
  //! Is the X Record extension used ?
  bool use_xrecord;

  //! XRecord context. Defines clients and events to capture.
  XRecordContext xrecord_context;

  //! X Connection for event capturing.
  Display *xrecord_datalink;

  // Event base for Xinput events
  static int xi_event_base;
#endif
};

#endif // X11INPUTMONITOR_HH
