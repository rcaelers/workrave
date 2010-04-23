// X11InputMonitor.cc --- ActivityMonitor for X11
//
// Copyright (C) 2001-2007, 2009, 2010 Rob Caelers <robc@krandor.nl>
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

#include "X11InputMonitor.hh"
#include "IInputMonitorListener.hh"

#include "timeutil.h"

#ifdef HAVE_APP_GTK
#include <gdk/gdk.h>
#endif

#include "Thread.hh"

using namespace std;

#ifdef HAVE_XRECORD
int X11InputMonitor::xi_event_base = 0;
#endif

static int (*old_handler)(Display *dpy, XErrorEvent *error);

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

//! Obtains the next X11 event with specified timeout.
static Bool
XNextEventTimed(Display* dsp, XEvent* event_return, long millis)
{
  if (millis == 0)
    {
      XNextEvent(dsp, event_return);
      return True;
    }

  struct timeval tv;
  tv.tv_sec = millis / 1000;
  tv.tv_usec = (millis % 1000) * 1000;

  XFlush(dsp);
  if (XPending(dsp))
    {
      XNextEvent(dsp, event_return);
      return True;
    }
  else
    {
      int fd = ConnectionNumber(dsp);
      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(fd, &readset);
      if (select(fd+1, &readset, NULL, NULL, &tv) <= 0)
        {
          return False;
        }
      else
        {
          if (XPending(dsp))
            {
              XNextEvent(dsp, event_return);
              return True;
            }
          else
            {
              return False;
            }
        }
    }
}


X11InputMonitor::X11InputMonitor(const string &display_name) :
  x11_display(NULL),
  abort(false)
{
#ifdef HAVE_XRECORD
  use_xrecord = false;
  xrecord_context = 0;
  xrecord_datalink = NULL;
#endif

  x11_display_name = display_name;
  monitor_thread = new Thread(this); // FIXME: LEAK
}


// FIXME: never executed....
X11InputMonitor::~X11InputMonitor()
{
  TRACE_ENTER("X11InputMonitor::~X11InputMonitor");
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
      delete monitor_thread;
    }

#ifdef HAVE_XRECORD
  if (xrecord_datalink != NULL)
    {
      XCloseDisplay(xrecord_datalink);
    }
#endif
  TRACE_EXIT();
}


bool
X11InputMonitor::init()
{
  monitor_thread->start();
  return true;
}

void
X11InputMonitor::terminate()
{
  TRACE_ENTER("X11InputMonitor::terminate");

#ifdef HAVE_XRECORD
  if (use_xrecord)
    {
      stop_xrecord();
    }
#endif

  abort = true;

#ifdef HAVE_XRECORD
  //FIXME:  stop_xrecord does not seem to work.
  if (use_xrecord)
    {
      monitor_thread->wait();
    }
#endif
  TRACE_EXIT();
}


void
X11InputMonitor::run()
{
  TRACE_ENTER("X11InputMonitor::run");

  if ((x11_display = XOpenDisplay(x11_display_name.c_str())) == NULL)
    {
      return;
    }

#ifdef HAVE_XRECORD
  run_xrecord();
#else
  run_events();
  XCloseDisplay(x11_display);
#endif

  TRACE_EXIT();
}

void
X11InputMonitor::run_events()
{
  TRACE_ENTER("X11InputMonitor::run_events");

  error_trap_enter();
  
  root_window = DefaultRootWindow(x11_display);
  set_all_events(root_window);

  XGrabButton(x11_display, AnyButton, AnyModifier, root_window, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  XSync(x11_display,False);


  error_trap_exit();
  
  Window lastMouseRoot = 0;
  while (1)
    {
      XEvent event;
      bool gotEvent = XNextEventTimed(x11_display, &event, 100);

      if (abort)
        {
          TRACE_EXIT();
          return;
        }

      if (gotEvent)
        {
          error_trap_enter();
          
          switch (event.xany.type)
            {
            case KeyPress:
              handle_keypress(&event);
              break;

            case CreateNotify:
              handle_create(&event);
              break;

            case ButtonPress:
            case ButtonRelease:
              handle_button(&event);
              break;
            }

          error_trap_exit();
        }

      
      // timeout
      Window root, child;
      int root_x, root_y, win_x, win_y;
      unsigned mask;

      error_trap_enter();
      
      XQueryPointer(x11_display, root_window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

      error_trap_exit();
      
      lastMouseRoot = root;
      fire_mouse(root_x, root_y);
    }

  TRACE_EXIT();
}

void
X11InputMonitor::set_event_mask(Window window)
{
  XWindowAttributes   attrs;
  unsigned long   events;
  Window    root,parent,*children;
  unsigned int    nchildren;
  char      *name;

  if (!XQueryTree(x11_display, window, &root, &parent, &children, &nchildren))
    return;

  if (XFetchName(x11_display, window, &name))
    {
      //printf("Watching: %s\n", name);
      XFree(name);
    }

  if (parent == None)
    {
      attrs.all_event_masks =
      attrs.do_not_propagate_mask = KeyPressMask;
    }
  else
    {
      XGetWindowAttributes(x11_display, window, &attrs);
    }

  events=((attrs.all_event_masks | attrs.do_not_propagate_mask) & KeyPressMask);

  XSelectInput(x11_display, window, SubstructureNotifyMask|PropertyChangeMask|EnterWindowMask|events);

  if (children)
  {
    while (nchildren)
      {
        set_event_mask(children[--nchildren]);
      }
    XFree(children);
  }
}


void
X11InputMonitor::set_all_events(Window window)
{
  error_trap_enter();
  
  set_event_mask(window);
  XSync(x11_display,False);

  error_trap_exit();
}


void
X11InputMonitor::handle_keypress(XEvent *event)
{
  (void) event;
  fire_keyboard(0, 0);
}


void
X11InputMonitor::handle_create(XEvent *event)
{
  set_all_events(event->xcreatewindow.window);
}


void
X11InputMonitor::handle_button(XEvent *event)
{
  (void) event;

  XSync(x11_display, 0);
  XAllowEvents(x11_display, ReplayPointer, CurrentTime);
  XSync(x11_display, 0);

  if (event != NULL)
    {
      int b = event->xbutton.button;
      // FIXME: this is a hack. XGrabButton does not generate a button release
      // event...

      fire_button(b, true);
      fire_button(b, false);
    }
  else
    {
      fire_action();
    }
}

void
X11InputMonitor::error_trap_enter()
{
#ifdef HAVE_APP_GTK
  gdk_error_trap_push();
#else
  old_handler = XSetErrorHandler(&errorHandler);
#endif
}

void
X11InputMonitor::error_trap_exit()
{
#ifdef HAVE_APP_GTK
  gdk_flush ();
  gdk_error_trap_pop();
#else
  XSetErrorHandler(old_handler);
#endif
}
  

#ifdef HAVE_XRECORD

void
X11InputMonitor::handle_xrecord_handle_key_event(XRecordInterceptData *data)
{
  (void) data;
  fire_keyboard(0, 0);
}

void
X11InputMonitor::handle_xrecord_handle_motion_event(XRecordInterceptData *data)
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
X11InputMonitor::handle_xrecord_handle_button_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;

  if (event != NULL)
    {
      int b = event->u.keyButtonPointer.state;

      fire_button(b, event->u.u.type == ButtonPress);
    }
  else
    {
      fire_action();
    }
}

void
X11InputMonitor::handle_xrecord_handle_device_key_event(XRecordInterceptData *data)
{
  TRACE_ENTER("X11InputMonitor::handle_xrecord_handle_device_key_event");
  deviceKeyButtonPointer *event = (deviceKeyButtonPointer *)data->data;
  static Time lastTime = 0;

  if (event->time != lastTime)
    {
      lastTime = event->time;
      fire_keyboard(0, 0);
    }
  
  TRACE_EXIT();
}

void
X11InputMonitor::handle_xrecord_handle_device_motion_event(XRecordInterceptData *data)
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
X11InputMonitor::handle_xrecord_handle_device_button_event(XRecordInterceptData *data)
{
  deviceKeyButtonPointer *event = (deviceKeyButtonPointer *)data->data;
  static Time lastTime = 0;

  if (event->time != lastTime)
    {
      lastTime = event->time;

      // FIXME: check if this is correct
      int b = event->state;
      
      fire_button(b, event->type == xi_event_base + XI_DeviceButtonPress);
    }
}

void
X11InputMonitor::handle_xrecord_callback(XPointer closure, XRecordInterceptData * data)
{
  xEvent *  event;
  X11InputMonitor *monitor = (X11InputMonitor *) closure;

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
              monitor->handle_xrecord_handle_device_key_event(data);
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


void
X11InputMonitor::run_xrecord()
{
  TRACE_ENTER("X11InputMonitor::run_xrecord");

  init_xrecord();

  if (use_xrecord &&
      XRecordEnableContext(xrecord_datalink, xrecord_context,  &handle_xrecord_callback, (XPointer)this))
    {
      xrecord_datalink = NULL;
    }
  else
    {
      TRACE_MSG("Fallback to run events");
      use_xrecord = false;
      run_events();
    }
  TRACE_EXIT();
}


//! Initialize the XRecord monitoring.
bool
X11InputMonitor::init_xrecord()
{
  TRACE_ENTER("X11InputMonitor::init_xrecord");
  use_xrecord = false;

  int major, minor;

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
X11InputMonitor::stop_xrecord()
{
  TRACE_ENTER("X11InputMonitor::stop_xrecord");
  if (use_xrecord)
    {
      XRecordDisableContext(xrecord_datalink, xrecord_context);
      XRecordFreeContext(x11_display, xrecord_context);
      XFlush(xrecord_datalink);
      XCloseDisplay(x11_display);
    }

  TRACE_EXIT();
  return true;
}

#endif
