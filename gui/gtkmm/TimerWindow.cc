// TimerWindow.cc --- Timer info Window
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <iostream>

#include "TimerWindow.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "Text.hh"

#include "TimerInterface.hh"
#include "ControlInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "Configurator.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#endif


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
TimerWindow::TimerWindow(GUI *g, ControlInterface *c) :
  core_control(c),
  gui(g)
{
}


//! Destructor.
TimerWindow::~TimerWindow()
{
  TRACE_ENTER("TimerWindow::~TimerWindow");

  if (timer_times != NULL)
    {
      delete [] timer_times;
    }

  if (timer_names != NULL)
    {
      delete [] timer_names;
    }

  TRACE_EXIT();
}



void
TimerWindow::init_widgets()
{
  timer_names = new Gtk::Widget*[GUIControl::BREAK_ID_SIZEOF];
  timer_times = new TimeBar*[GUIControl::BREAK_ID_SIZEOF];

  for (int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[count];
      Gtk::Image *img = manage(new Gtk::Image(timer->icon));
      Gtk::Widget *w;
      if (count == GUIControl::BREAK_ID_REST_BREAK)
	{
	  Gtk::Button *b = manage(new Gtk::Button());
	  b->set_relief(Gtk::RELIEF_NONE);
	  b->set_border_width(0);
	  b->add(*img);
	  b->signal_clicked().connect(SigC::slot(*this, &TimerWindow::on_menu_restbreak_now));
	  w = b;
	}
      else
	{
	  w = img;
	}
      
      timer_names[count] = w;
      
      timer_times[count] = manage(new TimeBar);
      timer_times[count]->set_text_alignment(1);
      timer_times[count]->set_progress(0, 60);
      timer_times[count]->set_text(_("Wait"));
    }
}


//! Updates the main window.
void
TimerWindow::update_widgets()
{
  bool node_master = true;
  int num_peers = 0;
  
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      node_master = dist_manager->is_master();
      num_peers = dist_manager->get_number_of_peers();
    }
#endif
  
  for (unsigned int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      TimerInterface *timer = GUIControl::get_instance()->timers[count].timer;
      TimeBar *bar = timer_times[count];

      if (!node_master && num_peers > 0)
        {
          bar->set_text(_("Inactive"));
          bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_progress(0, 60);
          bar->set_secondary_progress(0, 0);
          bar->update();
          continue;
        }
  
      if (timer == NULL)
        {
          continue;
        }
      
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
}



//! User requested immediate restbreak.
void
TimerWindow::on_menu_restbreak_now()
{
  gui->restbreak_now();
}



