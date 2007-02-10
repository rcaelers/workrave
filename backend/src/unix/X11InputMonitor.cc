// X11InputMonitor.cc --- ActivityMonitor for X11
//
// Copyright (C) 2001-2007 Rob Caelers <robc@krandor.org>
// All rights reserved.
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <math.h>

#include <stdio.h>
#include <sys/types.h>

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
#include <X11/Intrinsic.h>
#include <X11/Xos.h>
#include <X11/Xmu/Error.h>

#include "X11InputMonitor.hh"
#include "IInputMonitorListener.hh"

#include "timeutil.h"

#ifdef HAVE_APP_GTK
#include <gdk/gdk.h>
#endif

//! Intercepts X11 protocol errors.
static int
errorHandler(Display *dpy, XErrorEvent *error)
{
  if (error->error_code == BadWindow || error->error_code==BadDrawable)
    return 0;
  XmuPrintDefaultErrorMessage(dpy,error,stderr);
  return 0;
}


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


X11InputMonitor::X11InputMonitor(const char *name) :
  x11_display_name(NULL),
  x11_display(NULL),
  abort(false)
{
#ifdef HAVE_XRECORD
  use_xrecord = false;
  xrecord_context = 0;
  xrecord_datalink = NULL;
#endif
  if (name != NULL)
    {
      x11_display_name = strdup(name); // FIXME: LEAK
    }
  
  monitor_thread = new Thread(this); // FIXME: LEAK
}


// FIXME: never executed....
X11InputMonitor::~X11InputMonitor()
{
  TRACE_ENTER("X11InputMonitor::~X11InputMonitor");
  if (monitor_thread != NULL)
    {
      monitor_thread->stop();
      delete monitor_thread;
    }

  if (x11_display_name != NULL)
    {
      free(x11_display_name);
    }

#ifdef HAVE_XRECORD
  if (xrecord_datalink != NULL)
    {
      XCloseDisplay(xrecord_datalink);
    }
#endif
  TRACE_EXIT();
}


void
X11InputMonitor::init(IInputMonitorListener *l)
{
  this->listener = l;
  monitor_thread->start();
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

  TRACE_MSG("waiting");
  abort = true;

#ifdef HAVE_XRECORD
  //FIXME:  stop_xrecord does not seem to work.
  if (use_xrecord)
    {
      monitor_thread->stop();
    }
#endif
  //  wait_for_terminated_signal.wait();
  TRACE_EXIT();
}


void
X11InputMonitor::run()
{
  TRACE_ENTER("X11InputMonitor::run");

  if ((x11_display = XOpenDisplay(x11_display_name)) == NULL)
    {
      return;
    }

#ifdef HAVE_XRECORD
  run_xrecord();
#else
  run_events();
#endif
  
  wait_for_terminated_signal.signal();
  XCloseDisplay(x11_display);

  TRACE_EXIT();
}

void
X11InputMonitor::run_events()
{
  TRACE_ENTER("X11InputMonitor::run_events");

  root_window = DefaultRootWindow(x11_display);  

  set_all_events(root_window);

#ifdef HAVE_APP_GTK  
  gdk_error_trap_push();
#else
  int (*old_handler)(Display *dpy, XErrorEvent *error);
  old_handler = XSetErrorHandler(&errorHandler);
#endif
  
  XGrabButton(x11_display, AnyButton, AnyModifier, root_window, True,
              ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
  XSync(x11_display,False);

#ifdef HAVE_APP_GTK  
  gdk_error_trap_pop();
#else
  XSetErrorHandler(old_handler);
#endif
  
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
        }

      // timeout
      Window root, child;
      int root_x, root_y, win_x, win_y;
      unsigned mask;

      XQueryPointer(x11_display, root_window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);

      lastMouseRoot = root;
      listener->mouse_notify(root_x, root_y);
    }

  TRACE_EXIT();
}

void
X11InputMonitor::set_event_mask(Window window)
{
  XWindowAttributes 	attrs;
  unsigned long 	events;
  Window 		root,parent,*children;
  unsigned int 		nchildren;
  char			*name;

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

#ifdef HAVE_APP_GTK  
  gdk_error_trap_push();
#else          
  int (*old_handler)(Display *dpy, XErrorEvent *error);
  old_handler = XSetErrorHandler(&errorHandler);
#endif

  set_event_mask(window);
  XSync(x11_display,False);

#ifdef HAVE_APP_GTK  
  gdk_error_trap_pop();
#else
  XSetErrorHandler(old_handler);
#endif
}


void
X11InputMonitor::handle_keypress(XEvent *event)
{
  (void) event;
  //printf("Key pressed ");

#if 0
  char *key;

  key = XKeysymToString(XKeycodeToKeysym(x11_display, event->xkey.keycode, event->xkey.state));

  if (key != 0)
    {
      printf("%s\n",key);
    }
  else
    {
      key = XKeysymToString(XKeycodeToKeysym(x11_display, event->xkey.keycode, 0));
      
      if (key != 0)
        {
          printf("'%s'\n",key);
        }
      else
        {
          printf("(%u,%u)\n", event->xkey.state, event->xkey.keycode);
        }
    }
  
  fflush(stdout);
#endif
  // FIXME:
  listener->keyboard_notify(0, 0);
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
      
      listener->button_notify(b, true);
      listener->button_notify(b, false);
    }
  else
    {
      listener->action_notify();
    }
}



#ifdef HAVE_XRECORD

static void handleXRecordCallback(XPointer closure, XRecordInterceptData * data);

void
X11InputMonitor::handle_xrecord_handle_key_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;
  XKeyEvent  kevent;
  KeySym   keysym;
  char   buf[1];

  kevent.type = KeyPress;
  kevent.display = x11_display;
  kevent.state = event->u.keyButtonPointer.state;
  kevent.keycode = event->u.u.detail;
  XLookupString(&kevent, buf, sizeof(buf), &keysym, 0);
  listener->keyboard_notify(0, 0);
}

void
X11InputMonitor::handle_xrecord_handle_motion_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;

  if (event != NULL)
    {
      int x = event->u.keyButtonPointer.rootX;
      int y = event->u.keyButtonPointer.rootY;

      listener->mouse_notify(x, y, 0);
    }
  else
    {
      listener->action_notify();
    }
}


void
X11InputMonitor::handle_xrecord_handle_button_event(XRecordInterceptData *data)
{
  xEvent *event = (xEvent *)data->data;

  if (event != NULL)
    {
      int b = event->u.keyButtonPointer.state;

      listener->button_notify(b, event->u.u.type == ButtonPress);
    }
  else
    {
      listener->action_notify();
    }
}


static void
handleXRecordCallback(XPointer closure, XRecordInterceptData * data)
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
      XRecordEnableContext(xrecord_datalink, xrecord_context,  &handleXRecordCallback, (XPointer)this))
    {
      XRecordFreeContext(x11_display, xrecord_context);
      XCloseDisplay(xrecord_datalink);
      xrecord_datalink = NULL;
    }
  else
    {
      TRACE_MSG("Fallback to run events");
      use_xrecord = false;
      run_events();
    }
  TRACE_EXIT()
    }


//! Initialize the XRecord monitoring.
bool
X11InputMonitor::init_xrecord()
{
  TRACE_ENTER("X11InputMonitor::init_xrecord")
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
          recordRange->device_events.first = KeyPress;
          recordRange->device_events.last  = MotionNotify;

          // And create the XRECORD context.
          xrecord_context = XRecordCreateContext(x11_display, 0, &client,  1, &recordRange, 1);

          XFree(recordRange);
        }

      if (xrecord_context != 0)
        {
          XSync(x11_display, True);

          xrecord_datalink = XOpenDisplay(x11_display_name);
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
  TRACE_ENTER("X11InputMonitor::stop_xrecord")
  if (use_xrecord)
    {
      int ret = XRecordDisableContext(xrecord_datalink, xrecord_context);
      TRACE_MSG(ret);
    }
  
  TRACE_EXIT();
  return true;
}
 
#endif
