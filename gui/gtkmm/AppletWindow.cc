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

#include "Menus.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#endif

#include "eggtrayicon.h"

const string AppletWindow::CFG_KEY_APPLET = "gui/applet";
const string AppletWindow::CFG_KEY_APPLET_HORIZONTAL = "gui/applet/horizontal";
const string AppletWindow::CFG_KEY_APPLET_ENABLED = "gui/applet/enabled";
const string AppletWindow::CFG_KEY_APPLET_SHOW_MICRO_PAUSE = "gui/applet/show_micro_pause";
const string AppletWindow::CFG_KEY_APPLET_SHOW_REST_BREAK = "gui/applet/show_rest_break";
const string AppletWindow::CFG_KEY_APPLET_SHOW_DAILY_LIMIT = "gui/applet/show_daily_limit";


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow(GUI *g, ControlInterface *c) :
  TimerWindow(g, c),
  mode(APPLET_DISABLED),
  retry_init(false),
  applet_control(NULL)
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      show_break[i] = true;
    }

  Menus *menus = Menus::get_instance();
  menus->set_applet_window(this);
  
  plug = NULL;
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

  if (applet_enabled)
    {
      init_widgets();
      init_table();
      init_applet();
    }
  
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
AppletWindow::init_applet()
{
  TRACE_ENTER("AppletWindow::init_applet");
  if (!init_native_applet())
    {
      EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
      if (tray_icon != NULL)
        {
          plug = Glib::wrap(GTK_PLUG(tray_icon));
          plug->add(*this);
          plug->show_all();

          mode = APPLET_TRAY;
        }
    }
  else
    {
      mode = APPLET_GNOME;
    }
  TRACE_MSG(mode);
  TRACE_EXIT();
}


bool
AppletWindow::init_native_applet()
{
  CORBA_Environment ev;
  bool ok = true;
  
  bonobo_activate();

  CORBA_exception_init (&ev);
  applet_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_AppletControl",
                                            Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, NULL, &ev);
  
  if (applet_control == NULL || BONOBO_EX (&ev))
    {
      g_warning(_("Could not contact Workrave Panel"));
      ok = false;
    }
  

  long id = 0;

  if (ok)
    {
      id = GNOME_Workrave_AppletControl_get_socket_id(applet_control, &ev);

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
      plug = new Gtk::Plug(id);
      plug->add(*this);
      set_border_width(2);
      plug->show_all();
      
      plug->signal_delete_event().connect(SigC::slot(*this, &AppletWindow::delete_event));
      Menus *menus = Menus::get_instance();
      if (menus != NULL)
        {
          menus->resync_applet();
        }
    }
  
  CORBA_exception_free(&ev);
  return ok;
}

bool
AppletWindow::delete_event(GdkEventAny *event)
{
  TRACE_ENTER("AppletWindow::deleted");
  mode = APPLET_DISABLED;
  if (plug != NULL)
    {
      plug->remove();
      // FIXME: free memory
    }
  TRACE_EXIT();
  return true;
}
    

//! Updates the main window.
void
AppletWindow::fire()
{
  TRACE_ENTER("AppletWindow::fire");
  if (mode == APPLET_TRAY)
    {
      if (plug != NULL)
        {
          plug->remove();
          // FIXME: free memory
        }
      mode = APPLET_DISABLED;
    }
  
  if (mode == APPLET_DISABLED && applet_enabled)
    {
      TRACE_ENTER("AppletWindow::retrying");
      retry_init = true;
    }
  TRACE_EXIT();
}


//! Updates the main window.
void
AppletWindow::update()
{
  if (mode == APPLET_DISABLED)
    {
      if (retry_init)
        {
          init_applet();
          retry_init = false;
        }
    }
  else
    {
      update_widgets();
    }
}


void
AppletWindow::set_menu_active(int menu, bool active)
{
  TRACE_ENTER("AppletWindow::set_menu_active");
  CORBA_Environment ev;

  if (applet_control != NULL)
    {
      CORBA_exception_init (&ev);
      switch (menu)
        {
        case 0:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Normal", active, &ev);
          break;
        case 1:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Suspended", active, &ev);
          break;
        case 2:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Quiet", active, &ev);
          break;
        case 3:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/ShowLog", active, &ev);
          break;
        }
    }
  CORBA_exception_free(&ev);
  TRACE_EXIT();
}


bool
AppletWindow::get_menu_active(int menu)
{
  TRACE_ENTER("AppletWindow::get_menu_active");
  CORBA_Environment ev;
  bool ret = false;

  if (applet_control != NULL)
    {
      CORBA_exception_init (&ev);
      switch (menu)
        {
        case 0:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Normal", &ev);
          break;
        case 1:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Suspended", &ev);
          break;
        case 2:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Quiet", &ev);
          break;
        case 3:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/ShowLog", &ev);
          break;
        }
    }
  CORBA_exception_free(&ev);
  TRACE_EXIT();
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

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_ENABLED, &applet_enabled))
    {
      applet_enabled = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_ENABLED, applet_enabled);
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
