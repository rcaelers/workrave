// RestBreakWindow.cc --- window for the micropause
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
// All rights reserved.
//
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

const int TIMEOUT = 1000;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include <unistd.h>

#include "debug.hh"
#include "Util.hh"

const int MARGINX = 8;
const int MARGINY = 8;

// TODO: must be somewhere else.
#undef THREAD_PRIORITY_NORMAL
#undef DELETE
#undef OK
#undef ERROR

#include <gtkmm.h>

#include "RestBreakWindow.hh"
#include "TimeBar.hh"
#include "WindowHints.hh"
#include "BreakControl.hh"

#include "ActivityMonitorInterface.hh"
#include "TimerInterface.hh"
#include "ControlInterface.hh"


//! Constructor
/*!
 *  \param control The controller.
 */
RestBreakWindow::RestBreakWindow(bool ignorable) :
  window_width(0),
  window_height(0),
  core_control(NULL),
  timebar(NULL),
  progress_value(0),
  progress_max_value(0),
  insist_break(true)
{
  // Initialize this window
  set_border_width(12);

  // Title
  Gtk::HBox *info_box = manage(new Gtk::HBox(false, 6));
  string icon = Util::complete_directory("restbreak.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *info_img = manage(new Gtk::Image(icon));
  Gtk::Label *info_lab =
    manage(new Gtk::Label
           (_("This is your rest break. Make sure you stand up and\n"
              "walk away from your computer on a regular basis. Just\n"
              "walk around for a few minutes, stretch, and relax.")));

  info_box->pack_start(*info_img, false, false, 6);
  info_box->pack_start(*info_lab, false, true, 6);

  // Add other widgets.
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 6));
  vbox->pack_start(*info_box, false, false, 0);

  // Timebar
  timebar = manage(new TimeBar);
  vbox->pack_start(*timebar, false, false, 0);
  
  // Button box at the bottom.
  if (ignorable)
    {
      button_box = manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6));
      Gtk::Button *skipButton = manage(new Gtk::Button(_("Skip")));
      button_box->pack_end(*skipButton, Gtk::SHRINK, 0);
      Gtk::Button *postponeButton = manage(new Gtk::Button(_("Postpone")));
      button_box->pack_end(*postponeButton, Gtk::SHRINK, 0);
      
      GTK_WIDGET_UNSET_FLAGS(postponeButton->gobj(), GTK_CAN_FOCUS);
      GTK_WIDGET_UNSET_FLAGS(skipButton->gobj(), GTK_CAN_FOCUS);
      
      vbox->pack_end(*button_box, Gtk::SHRINK, 6);
  
      postponeButton->signal_clicked().connect(SigC::slot(*this, &RestBreakWindow::on_postpone_button_clicked));
      skipButton->signal_clicked().connect(SigC::slot(*this, &RestBreakWindow::on_skip_button_clicked));
    }
  
  add(*vbox);
  realize_if_needed();
  stick();
  
  // Set window hints.
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);
  WindowHints::set_always_on_top(Gtk::Widget::gobj(), true);
  
  add_events(Gdk::EXPOSURE_MASK);
  add_events(Gdk::FOCUS_CHANGE_MASK);
}


//! Destructor.
RestBreakWindow::~RestBreakWindow()
{
  TRACE_ENTER("RestBreakWindow::~RestBreakWindow");
  ungrab();
  TRACE_EXIT();
}


//! Starts the restbreak.
void
RestBreakWindow::start()
{
  refresh();
  center();
  show_all();
  set_avoid_pointer(false);

  if (insist_break)
    {
      grab();
    }

  present(); // After grab() please (Windows)
}


//! Stops the restbreak.
void
RestBreakWindow::stop()
{
  TRACE_ENTER("RestBreakWindow::stop");
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
RestBreakWindow::destroy()
{
  delete this;
}

//! The postpone button was clicked.
void
RestBreakWindow::on_postpone_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->postpone_break();
    }
}


//! The skip button was clicked.
void
RestBreakWindow::on_skip_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->skip_break();
    }
}


//! RestBreak window is realized.
void
RestBreakWindow::on_realize()
{
  // We need to call the base on_realize()
  Gtk::Window::on_realize();

  // Now we can allocate any additional resources we need
  Glib::RefPtr<Gdk::Window> window = get_window();

  // Alloc some colors
  Glib::RefPtr<Gdk::Colormap> colormap = get_colormap();
  border_color = Gdk::Color("black");
  colormap->alloc_color(border_color);

  window_gc = Gdk::GC::create(window);

  window->clear();
}


//! RestBreak window is exposed.
bool
RestBreakWindow::on_expose_event(GdkEventExpose* e)
{
  // Send event to parent.
  Gtk::Window::on_expose_event(e);

  draw_time_bar();
  return true;
}


//! Period timer callback.
void
RestBreakWindow::refresh()
{
  draw_time_bar();
}


void
RestBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}


void
RestBreakWindow::set_insist_break(bool insist)
{
  insist_break = insist;
}


//! Draws the timer bar.
void
RestBreakWindow::draw_time_bar()
{
  timebar->set_text_color(Gdk::Color("black"));
  timebar->set_progress(progress_value, progress_max_value);

  time_t time = progress_max_value - progress_value;
  char s[100];
  // FIXME: use TimeBar::time_to_string() ? In any case,
  // "avoid duplication of volatile information" :) MicroPauseWindow
  // has similar functionality.
  // FIXME: i18n
  sprintf(s, "Rest break for %02ld:%02ld minutes", time / 60, time % 60);
  
  timebar->set_text(s);
  timebar->update();
}
