// PreludeWindow.cc
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

#include "Text.hh"
#include "Util.hh"

#include "CoreFactory.hh"
#include "CoreInterface.hh"
#include "PreludeResponseInterface.hh"

#include "Dispatcher.hh"
#include "PreludeWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "TimeBar.hh"
#include "Hig.hh"


//! Construct a new Micropause window.
PreludeWindow::PreludeWindow(BreakId break_id)
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
  frame->set_border_width(6);
  frame->add(*hbox);
  frame->signal_flash().connect(SigC::slot(*this, &PreludeWindow::on_frame_flash));
  flash_visible = true;
  color_warn = Gdk::Color("orange");
  color_alert = Gdk::Color("red");
  add(*frame);

  switch (break_id)
    {
    case BREAK_ID_MICRO_PAUSE:
      label->set_markup(HigUtil::create_alert_text(_("Time for a micro-pause?"), NULL));
      break;
        
    case BREAK_ID_REST_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("You need a rest break..."), NULL));
      break;
          
    case BREAK_ID_DAILY_LIMIT:
      label->set_markup(HigUtil::create_alert_text(_("You should stop for today..."), NULL));
      break;

    default:
      break;
    }
  
  unset_flags(Gtk::CAN_FOCUS);

  stick();

  dispatcher = new Dispatcher;
  dispatch_connection = dispatcher->connect(SigC::slot_class(*this, &PreludeWindow::on_activity));
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
  dispatch_connection.disconnect();
  delete dispatcher;
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
  CoreInterface *core = CoreFactory::get_core();
  assert(core != NULL);
      
  core->set_activity_monitor_listener(NULL);
  
  delete this;
}


//! Stops the micropause.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  frame->set_frame_flashing(0);
  hide_all();

  TRACE_EXIT();
}


//! Refresh window.
void
PreludeWindow::refresh()
{
  time_bar->set_progress(progress_value, progress_max_value);

  string s;
  int tminus = progress_max_value - progress_value;
  if (tminus >= 0 || (tminus < 0 && flash_visible))
    {
      if (tminus < 0)
        tminus = 0;
      s = progress_text + " " + Text::time_to_string(tminus);
    }
  time_bar->set_text(s);
  time_bar->update();
}


void
PreludeWindow::set_progress(int value, int max_value)
{
  progress_value = value;
  progress_max_value = max_value;
  refresh();
}


void
PreludeWindow::set_progress_text(ProgressText text)
{
  switch (text)
    {
    case PROGRESS_TEXT_BREAK_IN:
      progress_text = _("Break in");
      break;
      
    case PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = _("Disappears in");
      break;
      
    case PROGRESS_TEXT_SILENT_IN:
      progress_text = _("Silent in");
      break;
    }
}


void
PreludeWindow::set_stage(Stage stage)
{
  const char *icon = NULL;
  switch(stage)
    {
    case STAGE_INITIAL:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;
      
    case STAGE_WARN:
      frame->set_frame_visible(true);
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_warn);
      icon = "prelude-hint-sad.png";
      break;
      
    case STAGE_ALERT:
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_alert);
      icon = "prelude-hint-sad.png";
      break;

    case STAGE_MOVE_OUT:
      if (! did_avoid_pointer())
        {
          int winx, winy;
          get_position(winx, winy);
          set_position(Gtk::WIN_POS_NONE);
          move (winx, SCREEN_MARGIN);
        }
      break;
    }
  if (icon != NULL)
    {
      string file = Util::complete_directory(icon, Util::SEARCH_PATH_IMAGES);
      image_icon->set(file);
    }
}


bool
PreludeWindow::delayed_stop()
{
  CoreInterface *core = CoreFactory::get_core();
  core->set_activity_monitor_listener(this);
  return true;
}


bool
PreludeWindow::action_notify()
{
  if (dispatcher != NULL)
    {
      dispatcher->send_notification();
    }
  return false; // false: kill listener.
}


void
PreludeWindow::on_activity()
{
  if (prelude_response != NULL)
    {
      prelude_response->prelude_stopped();
    }
}

void
PreludeWindow::on_frame_flash(bool frame_visible)
{
  flash_visible = frame_visible;
  refresh();
}
