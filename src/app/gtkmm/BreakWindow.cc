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

#include <gtkmm/window.h>
#include <gtkmm/stock.h>

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
  Gtk::Window(Gtk::WINDOW_TOPLEVEL),
#ifdef HAVE_X
  grab_wanted(false),
#endif
  grab_handle(NULL)
{
  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize();

  set_resizable(false);
  Gtk::Window::set_border_width(12);
  Glib::RefPtr<Gdk::Window> window = get_window();
  window->set_functions(Gdk::FUNC_MOVE);
  //  WindowHints::set_tool_window(Gtk::Widget::gobj(), true);

#ifdef HAVE_X
  GtkUtil::set_wmclass(*this, "Break");
#endif
}


//! Destructor.
BreakWindow::~BreakWindow()
{
  TRACE_ENTER("BreakWindow::~BreakWindow");
  ungrab();
  TRACE_EXIT();
}


//! Centers the window.
void
BreakWindow::center()
{
  GtkUtil::center_window(*this, head);
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
      ret = GtkUtil::create_image_button(_("Shut down"), "shutdown.png");
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
