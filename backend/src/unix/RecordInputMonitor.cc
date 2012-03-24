// RecordInputMonitor.cc --- ActivityMonitor for X11
//
// Copyright (C) 2001-2007, 2009, 2010, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <math.h>

#include <stdio.h>
#include <sys/types.h>

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

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

// Solaris needs this...
#define NEED_EVENTS

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xos.h>

#include "RecordInputMonitor.hh"

#include "Core.hh"
#include "ICore.hh"
#include "ICoreEventListener.hh"
#include "IInputMonitorListener.hh"

#include "timeutil.h"

#ifdef HAVE_APP_GTK
#include <gdk/gdkx.h>
#endif

#include "Thread.hh"

using namespace std;
using namespace workrave;

int RecordInputMonitor::xi_event_base = 0;

#ifndef HAVE_APP_GTK
static int (*old_handler)(Display *dpy, XErrorEvent *error);
#endif

#ifndef HAVE_APP_GTK
//! Intercepts X11 protocol errors.
static int
errorHandler(Display *dpy, XErrorEvent *error)
{
  (void)dpy;

  if (error->error_code == BadWindow || error->error_code==BadDrawable)
    return 0;
  return 0;
}
#endif

RecordInputMonitor::RecordInputMonitor(const string &display_name) :
  x11_display(NULL),
  abort(false)
{
  xrecord_context = 0;
  xrecord_datalink = NULL;

  x11_display_name = display_name;
  monitor_thread = new Thread(this);
}


RecordInputMonitor::~RecordInputMonitor()
{
  TRACE_ENTER("RecordInputMonitor::~RecordInputMonitor");
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
      delete monitor_thread;
    }

  if (xrecord_datalink != NULL)
    {
      XCloseDisplay(xrecord_datalink);
    }
  TRACE_EXIT();
}


bool
RecordInputMonitor::init()
{
  bool ok = init_xrecord();
  if (ok)
    {
      monitor_thread->start();
    }
  return ok;
}

void
RecordInputMonitor::terminate()
{
  TRACE_ENTER("RecordInputMonitor::terminate");

  stop_xrecord();

  abort = true;
  monitor_thread->wait();

  TRACE_EXIT();
}

void
RecordInputMonitor::run()
{
  TRACE_ENTER("RecordInputMonitor::run");

  error_trap_enter();

  if (XRecordEnableContext(xrecord_datalink, xrecord_context,  &handle_xrecord_callback, (XPointer)this))
    {
      error_trap_exit();
      xrecord_datalink = NULL;
    }
  
  TRACE_EXIT();
}

void
RecordInputMonitor::error_trap_enter()
{
#ifdef HAVE_APP_GTK
  gdk_error_trap_push();
#else
  old_handler = XSetErrorHandler(&errorHandler);
#endif
}

void
RecordInputMonitor::error_trap_exit()
{
#ifdef HAVE_APP_GTK
  gdk_flush ();
  gint err = gdk_error_trap_pop();
  (void) err;
#else
  XSetErrorHandler(old_handler);
#endif
}


void
RecordInputMonitor::handle_xrecord_handle_key_event(XRecordInterceptData *data)
{
  (void) data;
  fire_keyboard(false);
}

void
RecordInputMonitor::handle_xrecord_handle_motion_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;

  if (event != NULL)
    {
      int x = event->u.keyButtonPointer.rootX;
      int y = event->u.keyButtonPointer.rootY;

      fire_mouse(x, y, 0);
    }
  else
    {
      fire_action();
    }
}

void
RecordInputMonitor::handle_xrecord_handle_button_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;

  if (event != NULL)
    {
      fire_button(event->u.u.type == ButtonPress);
    }
  else
    {
      fire_action();
    }
}

void
RecordInputMonitor::handle_xrecord_handle_device_key_event(bool press, XRecordInterceptData *data)
{
  deviceKeyButtonPointer *event = (deviceKeyButtonPointer *)data->data;
  static Time lastTime = 0;
  static int detail = 0;
  static int state = 0;

  if (press)
    {
      if (event->time != lastTime)
        {
          lastTime = event->time;

          fire_keyboard(state == event->state && detail == event->detail);

          detail = event->detail;
          state = event->state;
        }
    }
  else
    {
      detail = 0;
      state = 0;
    }
}

void
RecordInputMonitor::handle_xrecord_handle_device_motion_event(XRecordInterceptData *data)
{
  deviceKeyButtonPointer *event = (deviceKeyButtonPointer *)data->data;
  static Time lastTime = 0;

  if (event->time != lastTime)
    {
      lastTime = event->time;
      int x = event->root_x;
      int y = event->root_y;

      fire_mouse(x, y, 0);
    }
}

void
RecordInputMonitor::handle_xrecord_handle_device_button_event(XRecordInterceptData *data)
{
  deviceKeyButtonPointer *event = (deviceKeyButtonPointer *)data->data;
  static Time lastTime = 0;

  if (event->time != lastTime)
    {
      lastTime = event->time;

      fire_button(event->type == xi_event_base + XI_DeviceButtonPress);
    }
}

void
RecordInputMonitor::handle_xrecord_callback(XPointer closure, XRecordInterceptData * data)
{
  xEvent *  event;
  RecordInputMonitor *monitor = (RecordInputMonitor *) closure;

  switch (data->category)
    {
    case XRecordStartOfData:
    case XRecordFromClient:
    case XRecordClientStarted:
    case XRecordClientDied:
    case XRecordEndOfData:
      break;

    case XRecordFromServer:
      event = (xEvent *)data->data;

      if (event->u.u.type == KeyPress)
        monitor->handle_xrecord_handle_key_event(data);
      else if (event->u.u.type == ButtonPress || event->u.u.type == ButtonRelease)
        monitor->handle_xrecord_handle_button_event(data);
      else if (event->u.u.type == MotionNotify)
        monitor->handle_xrecord_handle_motion_event(data);
      else if (xi_event_base != 0)
        {
          if (event->u.u.type == xi_event_base + XI_DeviceMotionNotify)
            {
              monitor->handle_xrecord_handle_device_motion_event(data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceKeyPress)
            {
              monitor->handle_xrecord_handle_device_key_event(true, data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceKeyRelease)
            {
              monitor->handle_xrecord_handle_device_key_event(false, data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceButtonPress ||
                   event->u.u.type == xi_event_base + XI_DeviceButtonRelease)
            {
              monitor->handle_xrecord_handle_device_button_event(data);
            }
        }
      break;
    }

  if (data != NULL)
    {
      XRecordFreeData(data);
    }
}


//! Initialize the XRecord monitoring.
bool
RecordInputMonitor::init_xrecord()
{
  TRACE_ENTER("RecordInputMonitor::init_xrecord");
  bool use_xrecord = false;
  int major, minor;

  if ((x11_display = XOpenDisplay(x11_display_name.c_str())) == NULL)
    {
      return false;
    }
  
  if (XRecordQueryVersion(x11_display, &major, &minor))
    {
      xrecord_context = 0;
      xrecord_datalink = NULL;
      use_xrecord = true;

      // Receive from ALL clients, including future clients.
      XRecordClientSpec client = XRecordAllClients;

      // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and
      // MotionNotify events.
      XRecordRange *recordRange = XRecordAllocRange();
      if (recordRange != NULL)
        {
          memset(recordRange, 0, sizeof(XRecordRange));

          int dummy = 0;
          Bool have_xi =  XQueryExtension(x11_display, "XInputExtension",
                                          &dummy, &xi_event_base, &dummy);

          if (have_xi && xi_event_base != 0)
            {
              TRACE_MSG("Using XI Events");
              recordRange->device_events.first = xi_event_base + XI_DeviceKeyPress;
              recordRange->device_events.last  = xi_event_base + XI_DeviceMotionNotify;
            }
          else
            {
              TRACE_MSG("Using Core Events");
              recordRange->device_events.first = KeyPress;
              recordRange->device_events.last  = MotionNotify;
            }

          // And create the XRECORD context.
          xrecord_context = XRecordCreateContext(x11_display, 0, &client,  1, &recordRange, 1);

          XFree(recordRange);
        }

      if (xrecord_context != 0)
        {
          XSync(x11_display, True);

          xrecord_datalink = XOpenDisplay(x11_display_name.c_str());
        }

      if (xrecord_datalink == NULL)
        {
          XRecordFreeContext(x11_display, xrecord_context);
          xrecord_context = 0;
          use_xrecord = false;
        }
    }

  TRACE_MSG("use_xrecord= " << use_xrecord);
  TRACE_EXIT();
  return use_xrecord;
}

//! Stop the XRecord activity monitoring.
bool
RecordInputMonitor::stop_xrecord()
{
  TRACE_ENTER("RecordInputMonitor::stop_xrecord");

  XRecordDisableContext(xrecord_datalink, xrecord_context);
  XRecordFreeContext(x11_display, xrecord_context);
  XFlush(xrecord_datalink);
  XCloseDisplay(x11_display);
  x11_display = NULL;

  TRACE_EXIT();
  return true;
}
