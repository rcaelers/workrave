// BreakWindow.cc --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

#include "preinclude.h"
#include "debug.hh"
#include "nls.h"

#include <gtkmm.h>
#include <math.h>

#include "BreakWindow.hh"
#include "GtkUtil.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "System.hh"


//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow() :
  Gtk::Window(Gtk::WINDOW_POPUP),
  SCREEN_MARGIN(20),
#ifdef HAVE_X
  grab_wanted(false),
#endif
  avoid_wanted(false),
  did_avoid(false),
  grab_handle(NULL),
  frame(NULL),
  border_width(0)
{
  Gtk::Window::set_border_width(0);

#ifdef HAVE_X
  GtkUtil::set_wmclass(*this, "Break");
#endif
}


//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");
#ifdef WIN32
  if (avoid_signal.connected())
    {
      avoid_signal.disconnect();
    }
#endif
  ungrab();
  TRACE_EXIT();
}


//! Adds a child to the window.
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


//! Sets the border width
void
BreakWindow::set_border_width(guint width)
{
  border_width = width;
  if (frame)
    frame->set_border_width(border_width);
}


//! Centers the window.
void
BreakWindow::center()
{
  TRACE_ENTER("BreakWindow::center");

  if (head.valid)
    {
      GtkRequisition size;
      size_request(&size);

#ifdef WIN32
      TRACE_MSG(
                head.geometry.get_width() << "x" << head.geometry.get_height() << " +" <<
                head.geometry.get_x() << "+" << head.geometry.get_y() << " " <<
                size.width << " " << size.height);
#else
      TRACE_MSG(
                head.monitor << " : " <<
                head.geometry.get_width() << "x" << head.geometry.get_height() << " +" <<
                head.geometry.get_x() << "+" << head.geometry.get_y() << " " <<
                size.width << " " << size.height);
#endif                
      
      int x = head.geometry.get_x() + (head.geometry.get_width() - size.width) / 2;
      int y = head.geometry.get_y() + (head.geometry.get_height() - size.height) / 2;
      
      set_position(Gtk::WIN_POS_NONE);
      move(x, y);
    }
  else
    {
      set_position(Gtk::WIN_POS_CENTER_ALWAYS);
    }
  TRACE_EXIT();
}


//! Centers the window
void
BreakWindow::set_screen(HeadInfo &head)
{
  this->head = head;
#ifdef HAVE_GTK_MULTIHEAD  
  Gtk::Window::set_screen(head.screen);
#endif
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


// Sets whether the window should run away for the mouse pointer.
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
  if (! is_realized())
    {
      Gdk::EventMask events;
      
      events = Gdk::ENTER_NOTIFY_MASK;
      if (avoid_pointer)
        {
          add_events(events);
          did_avoid = false;
        }
      else
        {
          set_events(get_events() & ~events);
        }
    }
#endif
}

#ifdef HAVE_X

//! GDK EventNotifyEvent notification.
bool
BreakWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  avoid_pointer((int)event->x, (int)event->y);
  return Gtk::Window::on_enter_notify_event(event);
}
#endif


//! Move window if pointer is neat specified location.
void
BreakWindow::avoid_pointer(int px, int py)
{
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

  int screen_height;

  if (head.valid)
    {
      screen_height = head.geometry.get_height();
    }
  else
    {
      screen_height = gdk_screen_height();
    }
  
  int top_y = SCREEN_MARGIN;
  int bottom_y = screen_height - height - SCREEN_MARGIN;
  if (winy < top_y + SCREEN_MARGIN)
    {
      winy = bottom_y;
    }
  else if (winy > bottom_y - SCREEN_MARGIN)
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
}

//! Creates the lock button
Gtk::Button *
BreakWindow::create_lock_button()
{
  Gtk::Button *ret;
  if (System::is_lockable())
    {
      ret = GtkUtil::create_image_button(_("Lock"), "lock.png");
      ret->signal_clicked()
        .connect(SigC::slot(*this, &BreakWindow::on_lock_button_clicked));
      GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
    }
  else
    {
      ret = NULL;
    }
  return ret;
}

//! Creates the lock button
Gtk::Button *
BreakWindow::create_shutdown_button()
{
  Gtk::Button *ret;
  if (System::is_shutdown_supported())
    {
      ret = GtkUtil::create_image_button(_("Shutdown"), "shutdown.png");
      ret->signal_clicked()
        .connect(SigC::slot(*this, &BreakWindow::on_shutdown_button_clicked));
      GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
    }
  else
    {
      ret = NULL;
    }
  return ret;
}

//! Creates the skip button.
Gtk::Button *
BreakWindow::create_skip_button()
{
  Gtk::Button *ret;
  ret = GtkUtil::create_custom_stock_button(_("Skip"), Gtk::Stock::CLOSE);
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
  return ret;
}


//! Creates the postpone button.
Gtk::Button *
BreakWindow::create_postpone_button()
{
  Gtk::Button *ret;
  ret = GtkUtil::create_custom_stock_button(_("Postpone"), Gtk::Stock::REDO);
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
  return ret;
}


#ifdef WIN32
bool
BreakWindow::on_avoid_pointer_timer()
{
  // gdk_window_get_pointer is not reliable.
  POINT p;
  if (GetCursorPos(&p))
    {
      avoid_pointer(p.x, p.y);
    }
  return true;
}

#endif

//! The lock button was clicked.
void
BreakWindow::on_lock_button_clicked()
{
  System::lock();
}

//! The lock button was clicked.
void
BreakWindow::on_shutdown_button_clicked()
{
  System::shutdown();
}
