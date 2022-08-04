// Copyright (C) 2001 - 2019 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#include "RecordInputMonitor.hh"

#include "debug.hh"

// Solaris needs this...
#define NEED_EVENTS

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xos.h>

#include "input-monitor/IInputMonitorListener.hh"

#if defined(HAVE_APP_GTK)
#include <gdk/gdk.h>
#if GDK_MAJOR_VERSION >= 4
#  include <gdk/x11/gdkx.h>
#else
#  include <gdk/gdkx.h>
#endif
#endif

#include <memory>
#include <chrono>

using namespace std;

int RecordInputMonitor::xi_event_base = 0;

#if !defined(HAVE_APP_GTK)
static int (*old_handler)(Display *dpy, XErrorEvent *error);
#endif

#if !defined(HAVE_APP_GTK)
//! Intercepts X11 protocol errors.
static int
errorHandler(Display *dpy, XErrorEvent *error)
{
  (void)dpy;

  if (error->error_code == BadWindow || error->error_code == BadDrawable)
    return 0;
  return 0;
}
#endif

RecordInputMonitor::RecordInputMonitor(const char *display_name)
  : x11_display_name(display_name)
  , x11_display(nullptr)
  , abort(false)
  , xrecord_context(0)
  , xrecord_datalink(nullptr)
{
}

RecordInputMonitor::~RecordInputMonitor()
{
  TRACE_ENTRY();
  if (monitor_thread != nullptr)
    {
      monitor_thread->join();
    }

  if (xrecord_datalink != nullptr)
    {
      XCloseDisplay(xrecord_datalink);
    }
}

bool
RecordInputMonitor::init()
{
  bool ok = init_xrecord();
  if (ok)
    {
      monitor_thread = std::make_shared<std::thread>([this] { run(); });
    }
  return ok;
}

void
RecordInputMonitor::terminate()
{
  TRACE_ENTRY();
  stop_xrecord();

  abort = true;
  monitor_thread->join();
}

void
RecordInputMonitor::run()
{
  TRACE_ENTRY();
  error_trap_enter();

  if (XRecordEnableContext(xrecord_datalink, xrecord_context, &handle_xrecord_callback, (XPointer)this))
    {
      error_trap_exit();
      xrecord_datalink = nullptr;
    }
}

void
RecordInputMonitor::error_trap_enter()
{
#if defined(HAVE_APP_GTK)
  gdk_x11_display_error_trap_push(gdk_display_get_default());
#else
  old_handler = XSetErrorHandler(&errorHandler);
#endif
}

void
RecordInputMonitor::error_trap_exit()
{
#if defined(HAVE_APP_GTK)
  gdk_display_flush(gdk_display_get_default());
  gdk_x11_display_error_trap_pop_ignored(gdk_display_get_default());
#else
  XSetErrorHandler(old_handler);
#endif
}

void
RecordInputMonitor::handle_xrecord_key_event(XRecordInterceptData *data)
{
  (void)data;
  fire_keyboard(false);
}

void
RecordInputMonitor::handle_xrecord_motion_event(XRecordInterceptData *data)
{
  TRACE_ENTRY();
  auto *event = (xEvent *)data->data;

  if (event != nullptr)
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
RecordInputMonitor::handle_xrecord_button_event(XRecordInterceptData *data)
{
  auto *event = (xEvent *)data->data;

  if (event != nullptr)
    {
      fire_button(event->u.u.type == ButtonPress);
    }
  else
    {
      fire_action();
    }
}

void
RecordInputMonitor::handle_xrecord_device_key_event(bool press, XRecordInterceptData *data)
{
  auto *event = (deviceKeyButtonPointer *)data->data;
  static Time last_time = 0;
  static int detail = 0;
  static int state = 0;

  if (press)
    {
      if (event->time != last_time)
        {
          last_time = event->time;

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
RecordInputMonitor::handle_xrecord_device_motion_event(XRecordInterceptData *data)
{
  auto *event = (deviceKeyButtonPointer *)data->data;
  static Time last_time = 0;

  if (event->time != last_time)
    {
      last_time = event->time;
      int x = event->root_x;
      int y = event->root_y;

      fire_mouse(x, y, 0);
    }
}

void
RecordInputMonitor::handle_xrecord_device_button_event(XRecordInterceptData *data)
{
  auto *event = (deviceKeyButtonPointer *)data->data;
  static Time last_time = 0;

  if (event->time != last_time)
    {
      last_time = event->time;

      fire_button(event->type == xi_event_base + XI_DeviceButtonPress);
    }
}

void
RecordInputMonitor::handle_xrecord_callback(XPointer closure, XRecordInterceptData *data)
{
  TRACE_ENTRY();
  xEvent *event;
  auto *monitor = (RecordInputMonitor *)closure;

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
        monitor->handle_xrecord_key_event(data);
      else if (event->u.u.type == ButtonPress || event->u.u.type == ButtonRelease)
        monitor->handle_xrecord_button_event(data);
      else if (event->u.u.type == MotionNotify)
        monitor->handle_xrecord_motion_event(data);
      else if (xi_event_base != 0)
        {
          TRACE_MSG("msg {} {}", (int)event->u.u.type, xi_event_base, XI_DeviceMotionNotify);
          if (event->u.u.type == xi_event_base + XI_DeviceMotionNotify)
            {
              monitor->handle_xrecord_device_motion_event(data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceKeyPress)
            {
              monitor->handle_xrecord_device_key_event(true, data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceKeyRelease)
            {
              monitor->handle_xrecord_device_key_event(false, data);
            }
          else if (event->u.u.type == xi_event_base + XI_DeviceButtonPress || event->u.u.type == xi_event_base + XI_DeviceButtonRelease)
            {
              monitor->handle_xrecord_device_button_event(data);
            }
        }
      break;
    }

  if (data != nullptr)
    {
      XRecordFreeData(data);
    }
}

//! Initialize the XRecord monitoring.
bool
RecordInputMonitor::init_xrecord()
{
  TRACE_ENTRY();
  bool use_xrecord = false;
  int major, minor;

  x11_display = XOpenDisplay(x11_display_name);

  if (x11_display != nullptr && XRecordQueryVersion(x11_display, &major, &minor))
    {
      xrecord_context = 0;
      xrecord_datalink = nullptr;
      use_xrecord = true;

      // Receive from ALL clients, including future clients.
      XRecordClientSpec client = XRecordAllClients;

      // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and
      // MotionNotify events.
      XRecordRange *recordRange = XRecordAllocRange();
      if (recordRange != nullptr)
        {
          memset(recordRange, 0, sizeof(XRecordRange));

          int opcode, error_base;
          Bool have_xi = XQueryExtension(x11_display, "XInputExtension", &opcode, &xi_event_base, &error_base);

          if (have_xi && xi_event_base != 0)
            {
              TRACE_MSG("Using XI Events");
              recordRange->device_events.first = xi_event_base + XI_DeviceKeyPress;
              recordRange->device_events.last = xi_event_base + XI_DeviceMotionNotify;
            }
          else
            {
              TRACE_MSG("Using Core Events");
              recordRange->device_events.first = KeyPress;
              recordRange->device_events.last = MotionNotify;
            }

          // And create the XRECORD context.
          xrecord_context = XRecordCreateContext(x11_display, 0, &client, 1, &recordRange, 1);

          XFree(recordRange);
        }

      if (xrecord_context != 0)
        {
          XSync(x11_display, True);

          xrecord_datalink = XOpenDisplay(x11_display_name);
        }

      if (xrecord_datalink == nullptr)
        {
          XRecordFreeContext(x11_display, xrecord_context);
          xrecord_context = 0;
          use_xrecord = false;
        }
    }

  TRACE_MSG("use_xrecord= {}", use_xrecord);
  return use_xrecord;
}

//! Stop the XRecord activity monitoring.
bool
RecordInputMonitor::stop_xrecord()
{
  TRACE_ENTRY();
  XRecordDisableContext(xrecord_datalink, xrecord_context);
  XRecordFreeContext(x11_display, xrecord_context);
  XFlush(xrecord_datalink);
  XCloseDisplay(x11_display);
  x11_display = nullptr;

  return true;
}
