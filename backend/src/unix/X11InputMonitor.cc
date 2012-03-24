// X11InputMonitor.cc --- ActivityMonitor for X11
//
// Copyright (C) 2001 - 2012 Rob Caelers <robc@krandor.nl>
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
  x11_display_name = display_name;
  monitor_thread = new Thread(this);
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

  abort = true;
  monitor_thread->wait();

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
  fire_keyboard(false);
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
      // FIXME: this is a hack. XGrabButton does not generate a button release
      // event...

      fire_button(true);
      fire_button(false);
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
  gint err = gdk_error_trap_pop();
  (void) err;
#else
  XSetErrorHandler(old_handler);
#endif
}
