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

#include "RemoteControl.hh"
#include "eggtrayicon.h"

const string AppletWindow::CFG_KEY_APPLET = "gui/applet";
const string AppletWindow::CFG_KEY_APPLET_HORIZONTAL = "gui/applet/vertical";
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
  applet_control(NULL),
  container(NULL),
  tray_menu(NULL),
  eventbox(NULL),
  timers_box(NULL),
  applet_size(0),
  reconfigure(false)
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      show_break[i] = true;
    }

  Menus *menus = Menus::get_instance();
  menus->set_applet_window(this);
  
  plug = NULL;
  applet_vertical = false;
  number_of_timers = 3;
  init();
}


//! Destructor.
AppletWindow::~AppletWindow()
{
  TRACE_ENTER("AppletWindow::~AppletWindow");

  if (timers_box != NULL)
    {
      delete timers_box;
    }
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
      init_applet();

      if (mode != APPLET_DISABLED)
        {
          init_table();
        }
    }
  
  TRACE_EXIT();
}
  

void
AppletWindow::init_table()
{
  TRACE_ENTER("AppletWindow::init_table");
  if (timers_box != NULL)
    {
      container->remove();
      delete timers_box;
      init_widgets();
    }

  int rows = number_of_timers;
  int columns = 1;
  
  if (applet_vertical)
    {
      plug->set_size_request(applet_size, -1);
    }
  else
    {
      GtkRequisition size;
      timer_times[0]->size_request(&size);
      rows = applet_height / size.height;

      if (rows <= 0)
        {
          rows = 1;
        }
      columns = (number_of_timers + rows - 1) / rows;
      
      plug->set_size_request(-1, applet_size);
    }

  TRACE_MSG("rows " << rows << " " << columns);
  timers_box = new Gtk::Table(rows, 2 * columns, false);
  timers_box->set_spacings(2);
  
  container->add(*timers_box);

  int count = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (show_break[i])
        {
          int cur_row = count % rows;
          int cur_col = count / rows;
          
          TRACE_MSG("row " << cur_row << " " << cur_col);
          if (applet_height != -1)
            {
              timer_times[i]->set_size_request(-1, applet_height / rows - 1 * (rows + 1) - 2);
            }
          
          timers_box->attach(*timer_names[i], 2 * cur_col, 2 * cur_col + 1, cur_row, cur_row + 1,
                             Gtk::FILL, Gtk::SHRINK);
          timers_box->attach(*timer_times[i], 2 * cur_col + 1, 2 * cur_col + 2, cur_row, cur_row + 1,
                             Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);

          count++;
        }
    }

  container->show_all();
  plug->show_all();
  TRACE_EXIT();
}


void
AppletWindow::init_applet()
{
  TRACE_ENTER("AppletWindow::init_applet");

  mode = APPLET_DISABLED;

  if (init_gnome_applet())
    {
      mode = APPLET_GNOME;
    }
  else
    {
      if (init_tray_applet())
        {
          mode = APPLET_TRAY;
        }
    }
  
  TRACE_EXIT();
}


void
AppletWindow::destroy_applet()
{
  TRACE_ENTER("AppletWindow::destroy_applet");

  if (mode == APPLET_GNOME)
    {
      destroy_gnome_applet();
    }
  else if (mode == APPLET_TRAY)
    {
      destroy_tray_applet();
    }
  
  TRACE_EXIT();
}


bool
AppletWindow::init_tray_applet()
{
  TRACE_ENTER("AppletWindow::init_tray_applet");
  bool ret = false;
  
  EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
  if (tray_icon != NULL)
    {
      eventbox = new Gtk::EventBox;
        
      // Necessary for popup menu 
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(SigC::slot(*this, &AppletWindow::on_button_press_event));
      container = eventbox;
      
      //eventbox->add(*timer_names[0]);
      plug = Glib::wrap(GTK_PLUG(tray_icon));
      plug->add(*eventbox);
      plug->show_all();
      
      // Tray menu
      if (tray_menu == NULL)
        {
          Menus *menus = Menus::get_instance();
          tray_menu = menus->create_tray_menu();
        }

      ret = true;
      applet_vertical = false;
      GtkRequisition req;
      plug->size_request(&req);
      applet_size = req.height;
      applet_height = req.height;
      applet_width = -1;
    }

  TRACE_EXIT();
  return ret;
}


void
AppletWindow::destroy_tray_applet()
{
  if (mode == APPLET_TRAY)
    {
      if (plug != NULL)
        {
          plug->remove(); // FIXME: free memory
          plug = NULL;
        }
      if (eventbox != NULL)
        {
          eventbox->remove();
          delete eventbox;
        }
    }
  mode = APPLET_DISABLED;
}


bool
AppletWindow::init_gnome_applet()
{
  TRACE_ENTER("AppletWindow::init_gnome_applet");
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

      applet_size = GNOME_Workrave_AppletControl_get_size(applet_control, &ev);
      applet_vertical =  GNOME_Workrave_AppletControl_get_vertical(applet_control, &ev);
      
      if (applet_vertical)
        {
          applet_width = applet_size;
          applet_height = -1;
        }
      else
        {
          applet_width = -1;
          applet_height = applet_size;
        }
      
      RemoteControl *remote = RemoteControl::get_instance();
      if (remote != NULL)
        {
          WorkraveControl *control = remote->get_remote_control();

          Bonobo_Unknown x = bonobo_object_corba_objref(BONOBO_OBJECT(control));
          GNOME_Workrave_AppletControl_register_control(applet_control, x, &ev);
        }
    }

  if (ok)
    {
      Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      frame->set_border_width(2);
      container = frame;
        
      plug = new Gtk::Plug(id);
      plug->add(*frame);
      plug->show_all();
      
      plug->signal_delete_event().connect(SigC::slot(*this, &AppletWindow::delete_event));
      Menus *menus = Menus::get_instance();
      if (menus != NULL)
        {
          menus->resync_applet();
        }
    }

  if (!ok)
    {
      applet_control = NULL;
    }
  
  CORBA_exception_free(&ev);
  TRACE_EXIT();
  return ok;
}


void
AppletWindow::destroy_gnome_applet()
{
  if (mode == APPLET_GNOME)
    {
      RemoteControl *remote = RemoteControl::get_instance();
      if (remote != NULL && applet_control != NULL)
        {
          CORBA_Environment ev;
          CORBA_exception_init (&ev);

          WorkraveControl *control = remote->get_remote_control();
          Bonobo_Unknown x = bonobo_object_corba_objref(BONOBO_OBJECT(control));
          
          GNOME_Workrave_AppletControl_unregister_control(applet_control, x, &ev);
          CORBA_exception_free(&ev);
        }
      
      if (plug != NULL)
        {
          plug->remove(); // FIXME: free memory.
        }
      if (frame != NULL)
        {
          frame->remove();
          delete frame;
        }
      applet_control = NULL; // FIXME: free memory.
    }
  mode = APPLET_DISABLED;
}



bool
AppletWindow::delete_event(GdkEventAny *event)
{
  TRACE_ENTER("AppletWindow::deleted");
  destroy_applet();
  return true;
}
    

//! Updates the main window.
void
AppletWindow::fire()
{
  TRACE_ENTER("AppletWindow::fire");
  if (mode == APPLET_TRAY)
    {
      destroy_tray_applet();
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
          if (mode != APPLET_DISABLED)
            {
              init_table();
            }
          retry_init = false;
        }
    }
  else
    {
      if (reconfigure)
        {
          init_table();
          reconfigure = false;
        }
      update_widgets();
    }
}


//! Users pressed some mouse button in the main window.
bool
AppletWindow::on_button_press_event(GdkEventButton *event)
{
  TRACE_ENTER("Applet::on_button_press_event");
  bool ret = false;

  if (tray_menu != NULL)
    {
      if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
        {
          tray_menu->popup(event->button, event->time);
          ret = true;
        }
    }
  
  TRACE_EXIT();
  return ret;
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
      CORBA_exception_free(&ev);
    }
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
      CORBA_exception_free(&ev);
    }
  TRACE_EXIT();
}


void
AppletWindow::set_applet_vertical(bool v)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_vertical", applet_vertical);
  applet_vertical = v;
  if (applet_vertical)
    {
      applet_width = applet_size;
      applet_height = -1;
    }
  else
    {
      applet_width = -1;
      applet_height = applet_size;
    }

  reconfigure = true;

  TRACE_EXIT();
}


void
AppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_size", size);
  applet_size = size;
  if (applet_vertical)
    {
      applet_width = applet_size;
      applet_height = -1;
    }
  else
    {
      applet_width = -1;
      applet_height = applet_size;
    }
  reconfigure = true;
  TRACE_EXIT();
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

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_HORIZONTAL, &applet_vertical))
    {
      applet_vertical = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_HORIZONTAL, applet_vertical);
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
