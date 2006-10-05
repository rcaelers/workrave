// X11InputMonitor.hh --- ActivityMonitor for X11
//
// Copyright (C) 2001, 2002, 2003, 2006 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2006-10-05 21:09:25 robc>
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
// $Id$
//

#ifndef X11INPUTMONITOR_HH
#define X11INPUTMONITOR_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <X11/X.h>
#include <X11/Xlib.h>

#ifdef HAVE_XRECORD
#include <X11/extensions/record.h>
#endif

#include "IInputMonitor.hh"

#include "Signal.hh"
#include "Thread.hh"


//! Activity monitor for a local X server.
class X11InputMonitor :
  public IInputMonitor,
  public Runnable
{
public:
  //! Constructor.
  X11InputMonitor(const char *display_name);

  //! Destructor.
  virtual ~X11InputMonitor();

  //! Initialize
  virtual void init(IInputMonitorListener *l);

  //! Terminate the monitor.
  virtual void terminate();

  //! The monitor's execution thread.
  virtual void run();

  //! the events execution thread.
  void run_events();

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
  char *x11_display_name;

  //! The X11 display handle.
  Display *x11_display;

  //! The X11 root window handle.
  Window root_window;

  //! The activity monitor thread.
  Thread *monitor_thread;

  //! Abort the main loop
  bool abort;
  
  //! Termination signal.
  Signal wait_for_terminated_signal;

  //! Where to deliver action events.
  IInputMonitorListener *listener;
  
#ifdef HAVE_XRECORD
  //! Is the X Record extension used ?
  bool use_xrecord;

  //! XRecord context. Defines clients and events to capture.
  XRecordContext xrecord_context;

  //! X Connection for event capturing.
  Display *xrecord_datalink;
#endif
};

#endif // X11INPUTMONITOR_HH
