// BreakWindow.cc --- base class for the break windows
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include <gtkmm.h>
#include <math.h>

#include "BreakWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"

const int MARGIN = 20;


//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow() :
  Gtk::Window (Gtk::WINDOW_POPUP),
#ifdef HAVE_X
  grab_wanted(false),
#endif
  avoid_wanted(false),
  did_avoid(false),
  frame(NULL),
  border_width(0),
  grab_handle(NULL)
{
  Gtk::Window::set_border_width(0);
}


//! Adds child
void
BreakWindow::add(Gtk::Widget& widget)
{
  if (! frame)
    {
      frame = manage(new Frame());
      frame->set_border_width(Gtk::Window::get_border_width());
      frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);
      set_border_width(border_width);
      Gtk::Window::add(*frame);
    }
  frame->add(widget);
}

//! Border 
void
BreakWindow::set_border_width(guint width)
{
  border_width = width;
  if (frame)
    frame->set_border_width(border_width);
}

//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");
  ungrab();
  TRACE_EXIT();
}



//! Centers window.
void
BreakWindow::center()
{
  TRACE_ENTER("BreakWindow::center");
  set_position(Gtk::WIN_POS_CENTER_ALWAYS);
  TRACE_EXIT();
}


//! Grabs the pointer and the keyboard.
bool
BreakWindow::grab()
{
  Glib::RefPtr<Gdk::Window> window = get_window();
  GdkWindow *gdkWindow = window->gobj();
#ifdef HAVE_X
  grab_wanted = true;
#endif
  if (! grab_handle)
    {
      grab_handle = WindowHints::grab(gdkWindow);
#ifdef HAVE_X
      if (! grab_handle)
	{
	  Glib::signal_timeout().connect(SigC::slot(*this, &BreakWindow::on_grab_retry_timer), 2000);
	}
#endif
    }
  return grab_handle != NULL;
}


//! Releases the pointer and keyboard grab
void
BreakWindow::ungrab()
{
#ifdef HAVE_X
  grab_wanted = false;
#endif
  if (grab_handle)
    {
      WindowHints::ungrab(grab_handle);
      grab_handle = NULL;
    }
}


#ifdef HAVE_X
//! Reattempt to get the grab
bool
BreakWindow::on_grab_retry_timer()
{
  if (grab_wanted)
    {
      return !grab();
    }
  else
    {
      return false;
    }
}
#endif


void
BreakWindow::set_avoid_pointer(bool avoid_pointer)
{
  avoid_wanted = avoid_pointer;
#ifdef WIN32
  if (avoid_pointer)
    {
      if (! avoid_signal.connected())
        {
          avoid_signal = Glib::signal_timeout()
            .connect(SigC::slot(*this, &BreakWindow::on_avoid_pointer_timer),
                     150);
        }
      did_avoid = false;
    }
  else
    {
      if (avoid_signal.connected())
        {
          avoid_signal.disconnect();
        }
    }
#else 
  Gdk::EventMask events;

  events = Gdk::ENTER_NOTIFY_MASK;
  if (avoid_pointer)
    {
      add_events(events);
    }
  else
    {
      set_events(get_events() & ~events);
    }
#endif
}

#ifdef HAVE_X

//! GDK EventNotifyEvent notification.
bool
BreakWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  avoid_pointer(event->x, event->y);
  return Gtk::Window::on_enter_notify_event(event);
}
#endif

void
BreakWindow::avoid_pointer(int px, int py)
{
  TRACE_ENTER_MSG("BreakWindow::avoid_pointer", "x="<<px<<",y="<<py);

  if (! avoid_wanted)
    return;
  
  Glib::RefPtr<Gdk::Window> window = get_window();
    
  int winx, winy, width, height, wind;
  window->get_geometry(winx, winy, width, height, wind);

#ifdef WIN32
  // This is only necessary for WIN32, since HAVE_X uses GdkEventCrossing.
  // Set gravitiy, otherwise, get_position() returns weird winy.
  set_gravity(Gdk::GRAVITY_STATIC); 
  get_position(winx, winy);
  if (px < winx || px > winx+width || py < winy || py > winy+height)
    return;
#else
  px += winx;
  py += winy;
#endif  

  int screen_height = gdk_screen_height();
  int top_y = MARGIN;
  int bottom_y = screen_height - height - MARGIN;
  if (winy < top_y + MARGIN)
    {
      winy = bottom_y;
    }
  else if (winy > bottom_y - MARGIN)
    {
      winy = top_y;
    }
  else
    {
      if (py > winy + height/2)
        {
          winy = top_y;
        }
      else
        {
          winy = bottom_y;
        }
    }

  set_position(Gtk::WIN_POS_NONE);
  move(winx, winy);
  did_avoid = true;
  TRACE_EXIT();
}

#ifdef WIN32
bool
BreakWindow::on_avoid_pointer_timer()
{
  TRACE_ENTER("BreakWindow::on_avoid_pointer_timer");

  // gdk_window_get_pointer is not reliable.
  POINT p;
  if (GetCursorPos(&p))
    {
      avoid_pointer(p.x, p.y);
    }
  return true;
}


#endif
