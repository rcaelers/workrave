// PreludeWindow.cc
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
#include "Util.hh"

// TODO: must be somewhere else.
#undef THREAD_PRIORITY_NORMAL
#undef DELETE
#undef OK
#undef ERROR


#include "PreludeWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "TimeBar.hh"


//! Construct a new Micropause window.
/*!
 *  \param control Interface to the controller.
 *  \param timer Interface to the restbreak timer.
 */
PreludeWindow::PreludeWindow()
{
  // Time bar
  time_bar = manage(new TimeBar);

  // Label
  label = manage(new Gtk::Label());
  
  // Box
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 6));
  vbox->pack_start(*label, false, false, 0);
  vbox->pack_start(*time_bar, false, false, 0);

  // Icon
  image_icon = manage(new Gtk::Image());
  
  // Box
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 6));
  hbox->pack_start(*image_icon, false, false, 0);
  hbox->pack_start(*vbox, false, false, 0);
  

  // Frame
  frame = manage(new Frame);
  frame->set_frame_style(Frame::STYLE_SOLID);
  frame->set_frame_width(6);
  frame->set_border_width(3);
  frame->add(*hbox);
  color_warn = Gdk::Color("orange");
  color_alert = Gdk::Color("red");
  add(*frame);

  unset_flags(Gtk::CAN_FOCUS);

  
  stick();
  
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
}



//! Starts the micropause.
void
PreludeWindow::start()
{
  TRACE_ENTER("PreludeWindow::start");
  
  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize_if_needed();

  // Set some window hints.
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);

  // Under Windows, Gtk::WINDOW_POPUP is always on top.
  // An additional always on top seems to give it focus, so don't do this.
#ifndef WIN32
  WindowHints::set_always_on_top(Gtk::Widget::gobj(), true);
#endif
  
  set_avoid_pointer(true);
  refresh();
  center();
  show_all();


  time_bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);

  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  PreludeWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
PreludeWindow::destroy()
{
  delete this;
}


//! Stops the micropause.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  frame->set_frame_flashing(0);
  set_avoid_pointer(false);
  hide_all();

  TRACE_EXIT();
}


//! Refresh window.
void
PreludeWindow::refresh()
{
  time_bar->update();
}


void
PreludeWindow::set_progress(int value, int max_value)
{
  TRACE_ENTER_MSG("PreludeWindow::set_progress", value << " " << max_value);
  time_bar->set_progress(value, max_value);
  string s = progress_text;
  s += TimeBar::time_to_string(max_value-value);
  time_bar->set_text(s);
  TRACE_EXIT()
}


void
PreludeWindow::set_text(string text)
{
  label->set_text(text);
}


void
PreludeWindow::set_progress_text(string text)
{
  progress_text = text;
}


void
PreludeWindow::set_frame(int stage)
{
  TRACE_ENTER_MSG("PreludeWindow::set_frame", stage);
  string icon;
  switch(stage)
    {
    case 0:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;
      
    case 1:
      {
        frame->set_frame_visible(true);
        frame->set_frame_flashing(500);
        frame->set_frame_color(color_warn);

        // temporary hack because enter_notify does not work under windows.
        int winx, winy;
        get_position(winx, winy);
        set_position(Gtk::WIN_POS_NONE);
        move (winx, 50);
      }
      icon = "prelude-hint-sad.png";
      break;
      
    case 2:
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_alert);
      icon = "prelude-hint-sad.png";
      break;
    }
  icon = Util::complete_directory(icon, Util::SEARCH_PATH_IMAGES);
  image_icon->set(icon);
  TRACE_EXIT();
}
