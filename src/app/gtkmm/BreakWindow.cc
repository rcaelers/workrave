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
#include <gtkmm/buttonbox.h>
#include <gtkmm/button.h>

#include <math.h>

#include "BreakWindow.hh"
#include "BreakResponseInterface.hh"
#include "GtkUtil.hh"
#include "WindowHints.hh"
#ifndef HAVE_BREAK_WINDOW_TITLEBAR
#include "Frame.hh"
#endif
#include "System.hh"


//! Constructor
/*!
 *  \param control The controller.
 */
BreakWindow::BreakWindow(BreakId break_id, HeadInfo &head,
                         bool ignorable, bool insist) :
#ifdef HAVE_BREAK_WINDOW_TITLEBAR
         Gtk::Window(Gtk::WINDOW_TOPLEVEL),
#else
         Gtk::Window(Gtk::WINDOW_POPUP),
#endif
         insist_break(insist),
         ignorable_break(ignorable),
#ifdef HAVE_X
         grab_wanted(false),
#endif
         grab_handle(NULL),
         break_response(NULL),
         gui(NULL)
{
  this->break_id = break_id;
  
#ifdef HAVE_X
  GtkUtil::set_wmclass(*this, "Break");
#endif

  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize();

  set_resizable(false);
  Glib::RefPtr<Gdk::Window> window = get_window();
  window->set_functions(Gdk::FUNC_MOVE);

  this->head = head;
#ifdef HAVE_GTK_MULTIHEAD  
  Gtk::Window::set_screen(head.screen);
#endif
}

//! Init GUI
void
BreakWindow::init_gui()
{
  if (gui == NULL)
    {
      gui = manage(create_gui());

      //#ifdef HAVE_BREAK_WINDOW_TITLEBAR
      set_border_width(12);
      add(*gui);
      //#else
//      set_border_width(0);
//      Frame *window_frame = manage(new Frame());
//      window_frame->set_border_width(12);
//      window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);
//      window_frame->add(*gui);
//      add(*window_frame);
//#endif      
      show_all_children();
      stick();
  
      // Set window hints.
      WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);
      WindowHints::set_always_on_top(Gtk::Widget::gobj(), true);

      // FIXME: check if it was intentionally not unset for RB
      if (break_id != BREAK_ID_REST_BREAK)
        {
          unset_flags(Gtk::CAN_FOCUS);
        }
    }
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
  ret->signal_clicked()
    .connect(SigC::slot(*this, &BreakWindow::on_skip_button_clicked));
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
  return ret;
}


//! Creates the postpone button.
Gtk::Button *
BreakWindow::create_postpone_button()
{
  Gtk::Button *ret;
  ret = GtkUtil::create_custom_stock_button(_("Postpone"), Gtk::Stock::REDO);
  ret->signal_clicked()
    .connect(SigC::slot(*this, &BreakWindow::on_postpone_button_clicked));
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

//! User has closed the main window.
bool
BreakWindow::on_delete_event(GdkEventAny *)
{
  on_postpone_button_clicked();
  return TRUE;
}

//! Break response
inline void
BreakWindow::set_response(BreakResponseInterface *bri)
{
  break_response = bri;
}

//! The postpone button was clicked.
void
BreakWindow::on_postpone_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->postpone_break(break_id);
    }
}



//! The skip button was clicked.
void
BreakWindow::on_skip_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->skip_break(break_id);
    }
}


//! Control buttons.
Gtk::HButtonBox *
BreakWindow::create_break_buttons(bool lockable,
                                  bool shutdownable)
{
  Gtk::HButtonBox *box = NULL;

  if (ignorable_break || lockable || shutdownable)
    {
      box = new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6);

      Gtk::Button *shutdown_button = NULL;
      if (shutdownable)
        {
          shutdown_button = manage(create_shutdown_button());
        }
      if (shutdown_button != NULL)
        {
          box->pack_end(*shutdown_button, Gtk::SHRINK, 0);
        }
      else if (lockable)
        {
          Gtk::Button *lock_button = manage(create_lock_button());
          if (lock_button != NULL)
            {
              box->pack_end(*lock_button, Gtk::SHRINK, 0);
            }
        }

      if (ignorable_break)
        {
          Gtk::Button *skip_button = manage(create_skip_button());
          box->pack_end(*skip_button, Gtk::SHRINK, 0);

          Gtk::Button *postpone_button = manage(create_postpone_button());
          box->pack_end(*postpone_button, Gtk::SHRINK, 0);
        }
    }

  return box;
}


//! Starts the daily limit.
void
BreakWindow::start()
{
  TRACE_ENTER("BreakWindow::start");

  init_gui();
  center();
  show_all();

  if (insist_break)
    {
      grab();
    }

#ifdef CAUSES_FVWM_FOCUS_PROBLEMS
  present(); // After grab() please (Windows)
#endif

  TRACE_EXIT();
}

//! Stops the daily limit.
void
BreakWindow::stop()
{
  TRACE_ENTER("BreakWindow::stop");

  ungrab();
  hide_all();

  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  BreakWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
BreakWindow::destroy()
{
  delete this;
}

//! Refresh
void
BreakWindow::refresh()
{
}

