// PreludeWindow.cc
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006 Rob Caelers & Raymond Penners
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

#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>

#include "debug.hh"
#include "nls.h"

#include "Text.hh"
#include "Util.hh"

#include "CoreFactory.hh"
#include "CoreInterface.hh"

#include "BreakResponseInterface.hh"
#include "PreludeWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "TimeBar.hh"
#include "Hig.hh"
#include "GtkUtil.hh"


//! Construct a new Microbreak window.
PreludeWindow::PreludeWindow(HeadInfo &head, BreakId break_id)
  : Gtk::Window(Gtk::WINDOW_POPUP),
    SCREEN_MARGIN(20),
    did_avoid(false),
    break_id(break_id),
    time_bar(NULL),
    frame(NULL),
    window_frame(NULL),
    label(NULL),
    image_icon(NULL),
    progress_value(0),
    progress_max_value(0),
    flash_visible(false),
    prelude_response(NULL)
{
  Gtk::Window::set_border_width(0);
#ifdef HAVE_X
  GtkUtil::set_wmclass(*this, "Prelude");
#endif

  init_avoid_pointer();
  realize();
  
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
  frame->signal_flash().connect(MEMBER_SLOT(*this, &PreludeWindow::on_frame_flash));
  flash_visible = true;
  color_warn = Gdk::Color("orange");
  color_alert = Gdk::Color("red");
  add(*frame);

  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("Time for a micro-break?"), NULL));
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

  show_all_children();
  stick();

  this->head = head;
#ifdef HAVE_GTK_MULTIHEAD  
  if (head.valid)
    {
      Gtk::Window::set_screen(head.screen);
    }
#endif
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
#ifdef WIN32
  if (avoid_signal.connected())
    {
      avoid_signal.disconnect();
    }
#endif
}



//! Starts the microbreak.
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
  
  refresh();
  GtkUtil::center_window(*this, head);
  show_all();


  time_bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);

  TRACE_EXIT();
}

//! Adds a child to the window.
void
PreludeWindow::add(Gtk::Widget& widget)
{
  if (! window_frame)
    {
      window_frame = manage(new Frame());
      window_frame->set_border_width(0);
      window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);
      Gtk::Window::add(*window_frame);
    }
  window_frame->add(widget);
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
  TRACE_ENTER("PreludeWindow::destroy");
  delete this;
  TRACE_EXIT();
}


//! Stops the microbreak.
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
  char s[128];
      
  time_bar->set_progress(progress_value, progress_max_value);

  int tminus = progress_max_value - progress_value;
  if (tminus >= 0 || (tminus < 0 && flash_visible))
    {
      if (tminus < 0)
        tminus = 0;

      sprintf(s, progress_text.c_str(), Text::time_to_string(tminus).c_str());
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
PreludeWindow::set_progress_text(AppInterface::PreludeProgressText text)
{
  switch (text)
    {
    case AppInterface::PROGRESS_TEXT_BREAK_IN:
      progress_text = _("Break in %s");
      break;
      
    case AppInterface::PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = _("Disappears in %s");
      break;
      
    case AppInterface::PROGRESS_TEXT_SILENT_IN:
      progress_text = _("Silent in %s");
      break;
    }
}


void
PreludeWindow::set_stage(AppInterface::PreludeStage stage)
{
  const char *icon = NULL;
  switch(stage)
    {
    case AppInterface::STAGE_INITIAL:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;
      
    case AppInterface::STAGE_WARN:
      frame->set_frame_visible(true);
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_warn);
      icon = "prelude-hint-sad.png";
      break;
      
    case AppInterface::STAGE_ALERT:
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_alert);
      icon = "prelude-hint-sad.png";
      break;

    case AppInterface::STAGE_MOVE_OUT:
      if (! did_avoid)
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


void
PreludeWindow::on_frame_flash(bool frame_visible)
{ 
  TRACE_ENTER("PreludeWindow::on_frame_flash");
  flash_visible = frame_visible;
  refresh();
  TRACE_EXIT();
}

// Sets whether the window should run away for the mouse pointer.
void
PreludeWindow::init_avoid_pointer()
{
#ifdef WIN32
  if (! avoid_signal.connected())
    {
      avoid_signal = Glib::signal_timeout()
        .connect(MEMBER_SLOT(*this, &PreludeWindow::on_avoid_pointer_timer),
                 150);
    }
#else
  if (! is_realized())
    {
      Gdk::EventMask events;
      
      events = Gdk::ENTER_NOTIFY_MASK;
      add_events(events);
    }
#endif
  did_avoid = false;
}

#ifdef HAVE_X

//! GDK EventNotifyEvent notification.
bool
PreludeWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  avoid_pointer((int)event->x, (int)event->y);
  return Gtk::Window::on_enter_notify_event(event);
}
#endif


//! Move window if pointer is neat specified location.
void
PreludeWindow::avoid_pointer(int px, int py)
{
  TRACE_ENTER_MSG("PreludeWindow::avoid_pointer" << px << " " << py);
  Glib::RefPtr<Gdk::Window> window = get_window();
    
  int winx, winy, width, height, wind;
  window->get_geometry(winx, winy, width, height, wind);

  TRACE_MSG("geom" << winx << " " << winy << " " << width << " " << height << " ");

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

  int screen_height = head.get_height();
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

#ifdef WIN32
bool
PreludeWindow::on_avoid_pointer_timer()
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
