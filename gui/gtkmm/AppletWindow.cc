// AppletWindow.cc --- Main info Window
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

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <iostream>

#include "AppletWindow.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "Text.hh"

#include "TimerInterface.hh"
#include "ControlInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "eggtrayicon.h"

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow(GUI *g, ControlInterface *c) :
  core_control(c),
  gui(g)
{
  init();
}


//! Destructor.
AppletWindow::~AppletWindow()
{
  TRACE_ENTER("AppletWindow::~AppletWindow");
  TRACE_EXIT();
}


//! Initializes the main window.
void
AppletWindow::init()
{
  TRACE_ENTER("AppletWindow::init");

  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 0));
  
  GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[0];
  Gtk::Image *img = manage(new Gtk::Image(timer->icon));
  bar = manage(new TimeBar);
  
  hbox->pack_start(*img, true, true, 0);
  hbox->pack_start(*bar, true, true, 0);
  
  bar->set_text_alignment(1);
  bar->set_progress(0, 60);
  bar->set_text(_("Wait"));
  
  add(*hbox);

  const char *plugid = getenv("WORKRAVE_PLUG");
  if (plugid != NULL)
    {
      TRACE_MSG("plug " << atoi(plugid));
      Gtk::Plug *plug = new Gtk::Plug(atoi(plugid));
      plug->add(*this);
      
      plug->show_all();
    }
  else
    {
      EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
      if (tray_icon != NULL)
        {
          Gtk::Container *tray = Glib::wrap(GTK_CONTAINER(tray_icon));
          tray->add(*this);
          tray->show_all();
        }
    }
  
  TRACE_EXIT();
}




//! Updates the main window.
void
AppletWindow::update()
{
  TimerInterface *timer = GUIControl::get_instance()->timers[0].timer;

  TimerInterface::TimerState timerState = timer->get_state();

  // Collect some data.
  time_t maxActiveTime = timer->get_limit();
  time_t activeTime = timer->get_elapsed_time();
  time_t breakDuration = timer->get_auto_reset();
  time_t idleTime = timer->get_elapsed_idle_time();
  bool overdue = (maxActiveTime < activeTime);
  
  // Set the text
  if (timer->is_limit_enabled() && maxActiveTime != 0)
    {
      bar->set_text(Text::time_to_string(maxActiveTime - activeTime));
    }
  else
    {
      bar->set_text(Text::time_to_string(activeTime));
    }
  
  // And set the bar.
  bar->set_secondary_progress(0, 0);
  
  if (timerState == TimerInterface::STATE_INVALID)
    {
      bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
      bar->set_progress(0, 60);
      bar->set_text(_("Wait"));
    }
  else
    {
      // Timer is running, show elapsed time.
      bar->set_progress(activeTime, maxActiveTime);
      
      if (overdue)
        {
          bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
        }
      else
        {
          bar->set_bar_color(TimeBar::COLOR_ID_ACTIVE);
        }
      
      if (//timerState == TimerInterface::STATE_STOPPED &&
          timer->is_auto_reset_enabled() && breakDuration != 0)
        {
          // resting.
          bar->set_secondary_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_secondary_progress(idleTime, breakDuration);
        }
    }
  bar->update();
}


//! User has closed the main window.
bool
AppletWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTER("AppletWindow::on_delete_event");
  TRACE_EXIT();
  return true;
}
