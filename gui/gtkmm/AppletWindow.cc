// AppletWindow.cc --- Applet info Window
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

#include "AppletWindow.hh"
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

#include "eggtrayicon.h"

const string AppletWindow::CFG_KEY_APPLET = "gui/applet";
const string AppletWindow::CFG_KEY_APPLET_HORIZONTAL = "gui/applet/horizontal";
const string AppletWindow::CFG_KEY_APPLET_SHOW_MICRO_PAUSE = "gui/applet/show_micro_pause";
const string AppletWindow::CFG_KEY_APPLET_SHOW_REST_BREAK = "gui/applet/show_rest_break";
const string AppletWindow::CFG_KEY_APPLET_SHOW_DAILY_LIMIT = "gui/applet/show_daily_limit";


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow(GUI *g, ControlInterface *c) :
  core_control(c),
  gui(g)
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      show_break[i] = true;
    }

  horizontal = true;
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
  read_configuration();
  init_widgets();
  init_table();
  init_applet();
  
  TRACE_EXIT();
}
  

void
AppletWindow::init_table()
{
  if (horizontal)
    {
      timers_box = manage(new Gtk::Table(2 * GUIControl::BREAK_ID_SIZEOF, 1, false));
    }
  else
    {
      timers_box = manage(new Gtk::Table(GUIControl::BREAK_ID_SIZEOF, 2, false));
    }

  timers_box->set_spacings(2);

  int count = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (show_break[i])
        {
          if (horizontal)
            {
              timers_box->attach(*timer_names[i], 2 * count, 2 * count + 1, 0, 1, Gtk::FILL);
              timers_box->attach(*timer_times[i], 2 * count + 1, 2 * count + 2, 0, 1,
                                 Gtk::EXPAND | Gtk::FILL);
            }
          else
            {
              timers_box->attach(*timer_names[i], 0, 1, count, count + 1, Gtk::FILL);
              timers_box->attach(*timer_times[i], 1, 2, count, count + 1, Gtk::EXPAND | Gtk::FILL);
            }
          count++;
        }
    }
  
  add(*timers_box);
}


void
AppletWindow::init_widgets()
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
	  //b->signal_clicked().connect(SigC::slot(*this, &AppletWindow::on_menu_restbreak_now));
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


void
AppletWindow::init_applet()
{
  if (!init_native_applet())
    {
      EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
      if (tray_icon != NULL)
        {
          Gtk::Container *tray = Glib::wrap(GTK_CONTAINER(tray_icon));
          tray->add(*this);
          tray->show_all();
        }
    }
}


bool
AppletWindow::init_native_applet()
{
  GNOME_Workrave_AppletControl ctrl;
  CORBA_Environment ev;
  bool ok = true;
  
  bonobo_activate();

  CORBA_exception_init (&ev);
  ctrl = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_AppletControl",
                                            Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, NULL, &ev);
  
  if (ctrl == NULL || BONOBO_EX (&ev))
    {
      g_warning(_("Could not contact Workrave Panel"));
      ok = false;
    }
  

  long id = 0;

  if (ok)
    {
      id = GNOME_Workrave_AppletControl_get_socket_id(ctrl, &ev);

      if (BONOBO_EX (&ev))
        {
          char *err = bonobo_exception_get_text(&ev);
          g_warning (_("An exception occured '%s'"), err);
          g_free(err);
          ok = false;
        }
    }

  if (ok)
    {
      Gtk::Plug *plug = new Gtk::Plug(id);
      plug->add(*this);
      set_border_width(2);
      plug->show_all();
    }

  CORBA_exception_free(&ev);
  return ok;
}


//! Updates the main window.
void
AppletWindow::update()
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
          // FIXME: error handling.
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

  return;
}



//! User requested immediate restbreak.
void
AppletWindow::on_menu_restbreak_now()
{
  gui->restbreak_now();
}



//! User has closed the main window.
bool
AppletWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTER("AppletWindow::on_delete_event");
  TRACE_EXIT();
  return true;
}

void
AppletWindow::read_configuration()
{
  bool b;

  Configurator *c = GUIControl::get_instance()->get_configurator();

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_HORIZONTAL, &horizontal))
    {
      horizontal = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_HORIZONTAL, horizontal);
    }

  bool rc;
  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_SHOW_MICRO_PAUSE, &rc))
    {
      rc = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_SHOW_MICRO_PAUSE, rc);
    }
  show_break[GUIControl::BREAK_ID_MICRO_PAUSE] = rc;
  
  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_SHOW_REST_BREAK, &rc))
    {
      rc = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_SHOW_REST_BREAK, rc);
    }
  show_break[GUIControl::BREAK_ID_REST_BREAK] = rc;

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_SHOW_DAILY_LIMIT, &rc))
    {
      rc = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_SHOW_DAILY_LIMIT, rc);
    }
  show_break[GUIControl::BREAK_ID_DAILY_LIMIT] = rc;
}
