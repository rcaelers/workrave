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

#ifdef HAVE_GNOME
#include "RemoteControl.hh"
#endif
#include "eggtrayicon.h"

const string AppletWindow::CFG_KEY_APPLET = "gui/applet";
const string AppletWindow::CFG_KEY_APPLET_HORIZONTAL = "gui/applet/vertical";
const string AppletWindow::CFG_KEY_APPLET_ENABLED = "gui/applet/enabled";
const string AppletWindow::CFG_KEY_APPLET_CYCLE_TIME = "gui/applet/cycle_time";
const string AppletWindow::CFG_KEY_APPLET_POSITION = "/position";
const string AppletWindow::CFG_KEY_APPLET_FLAGS = "/flags";
const string AppletWindow::CFG_KEY_APPLET_IMMINENT = "/imminent";



//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow(GUI *g, ControlInterface *c) :
  TimerWindow(g, c),
  mode(APPLET_DISABLED),
  retry_init(false),
  container(NULL),
  timers_box(NULL),
  tray_menu(NULL),
  eventbox(NULL),
  frame(NULL),
  cycle_time(10),
  applet_size(0),
#ifdef HAVE_GNOME
  applet_control(NULL),
#endif
  reconfigure(false)
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_position[i] = i;
      break_flags[i] = 0;
      break_imminent_time[i] = 0;
      
      for (int j = 0; j < GUIControl::BREAK_ID_SIZEOF; j++)
        {
          break_slots[i][j] = -1;
        }
      break_slot_cycle[i] = 0;
    }
  
  Menus *menus = Menus::get_instance();
#ifdef HAVE_GNOME
  menus->set_applet_window(this);
#endif
  
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

  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      config->add_listener(AppletWindow::CFG_KEY_APPLET, this);
    }
  
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

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      init_slot(i);
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
      int cycle = break_slot_cycle[i];
      int id = break_slots[i][cycle];

      
        
      if (id != -1)
        {
          int cur_row = count % rows;
          int cur_col = count / rows;
          
          TRACE_MSG("row " << cur_row << " " << cur_col);
          if (applet_height != -1)
            {
              timer_times[id]->set_size_request(-1, applet_height / rows - 1 * (rows + 1) - 2);
            }
          
          timers_box->attach(*timer_names[id], 2 * cur_col, 2 * cur_col + 1, cur_row, cur_row + 1,
                             Gtk::FILL, Gtk::SHRINK);
          timers_box->attach(*timer_times[id], 2 * cur_col + 1, 2 * cur_col + 2, cur_row, cur_row + 1,
                             Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);

          count++;
        }
    }

  container->show_all();
  plug->show_all();
  TRACE_EXIT();
}


void
AppletWindow::init_slot(int slot)
{
  TRACE_ENTER_MSG("AppletWindow::init_slot", slot);
  int count = 0;
  int breaks_id[GUIControl::BREAK_ID_SIZEOF];
  bool stop = false;

  // Collect all timers for this slot.
  for (int i = 0; !stop && i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (break_position[i] == slot)
        {
          breaks_id[count] = i;
          break_flags[count] &= ~BREAK_SKIP;
          TRACE_MSG("1 " << count << " " << i << " " << break_flags[i]);
          count++;
        }
    }

  // Compute timer that will elapse first.
  time_t first = 0;
  int first_id = -1;
    
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      TimerInterface *timer = GUIControl::get_instance()->timers[id].timer;
      time_t time_left = timer->get_limit() - timer->get_elapsed_time();
        
      // Exclude break if not imminent.
      if (flags & BREAK_WHEN_IMMINENT && time_left > break_imminent_time[id])
        {
          break_flags[id] |= BREAK_SKIP;
          TRACE_MSG("2 skipping " <<  i << " " << id);
        }

      // update first imminent timer.
      if (!(flags & BREAK_SKIP) && (first_id == -1 || time_left < first))
        {
          first_id = id;
          first = time_left;
        }
    }

  
  // Exclude break if not first.
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_WHEN_FIRST && first_id != id)
            {
              break_flags[id] |= BREAK_SKIP;
              TRACE_MSG("3 skipping " << i << " " << id);
            }
        }
    }

  
  // Exclude breaks if not exclusive.
  bool have_one = false;
  int breaks_left = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_EXCLUSIVE && have_one)
            {
              break_flags[id] |= BREAK_SKIP;
              TRACE_MSG("4 skipping " <<  i << " " << id);
            }

          have_one = true;
        }
      
      if (!(flags & BREAK_SKIP))
        {
          breaks_left++;
        }
    }

  TRACE_MSG("4a left " <<  breaks_left);
  
  if (breaks_left == 0)
    {
      for (int i = 0; i < count; i++)
        {
          int id = breaks_id[i];
          int flags = break_flags[id];
          
          if (flags & BREAK_DEFAULT && flags & BREAK_SKIP)
            {
              break_flags[id] &= ~BREAK_SKIP;
              TRACE_MSG("5 unskipping " <<  i << " " << id);
              breaks_left = 1;
              break;
            }
        }
    }

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slots[slot][i] = -1;
    }

  int new_count = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];
          
      TRACE_MSG("6 slot " <<  slot << " " << count << " " << id);
      if (!(flags & BREAK_SKIP))
        {
          break_slots[slot][new_count] = id;
          TRACE_MSG("7 slot " <<  slot << " " << new_count << " " << id);
          new_count++;
        }
    }

  TRACE_EXIT();
}


void
AppletWindow::cycle_slots()
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i]++;
      if (break_slot_cycle[i] >= GUIControl::BREAK_ID_SIZEOF
          || break_slots[i][break_slot_cycle[i]] == -1)
        {
          break_slot_cycle[i] = 0;
        }
    }
}

void
AppletWindow::init_applet()
{
  TRACE_ENTER("AppletWindow::init_applet");

  mode = APPLET_DISABLED;

#ifdef HAVE_GNOME  
  if (init_gnome_applet())
    {
      mode = APPLET_GNOME;
    }
  else
#endif    
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

  if (mode == APPLET_TRAY)
    {
      destroy_tray_applet();
    }  
#ifdef HAVE_GNOME
  else if (mode == APPLET_GNOME)
    {
      destroy_gnome_applet();
    }
#endif  
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

#ifdef HAVE_GNOME
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
#endif


bool
AppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  TRACE_ENTER("AppletWindow::deleted");
  destroy_applet();
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
      destroy_tray_applet();
    }
  
  if (mode == APPLET_DISABLED && applet_enabled)
    {
      TRACE_MSG("AppletWindow::retrying");
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
      time_t t = time(NULL);
      if (t % cycle_time == 0)
        {
          init_table();
          cycle_slots();
        }
      
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


#ifdef HAVE_GNOME
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
  return ret;
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
#endif
  

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
  Configurator *c = GUIControl::get_instance()->get_configurator();

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_ENABLED, &applet_enabled))
    {
      applet_enabled = true;
      c->set_value(AppletWindow::CFG_KEY_APPLET_ENABLED, applet_enabled);
    }

  if (!c->get_value(AppletWindow::CFG_KEY_APPLET_CYCLE_TIME, &cycle_time))
    {
      cycle_time = 10;
      c->set_value(AppletWindow::CFG_KEY_APPLET_CYCLE_TIME, cycle_time);
    }
  
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      GUIControl::TimerData &data = GUIControl::get_instance()->timers[i];
      
      int value = 0;
      
      if (!c->get_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_POSITION, &value))
        {
          value = i;
          c->set_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_POSITION, value);
        }
      break_position[i] = value;

      if (!c->get_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_FLAGS, &value))
        {
          value = BREAK_EXCLUSIVE;
          c->set_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_FLAGS, value);
        }
      break_flags[i] = value;
      
      if (!c->get_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_IMMINENT, &value))
        {
          value = 30;
          c->set_value(CFG_KEY_APPLET + "/" + data.break_name + CFG_KEY_APPLET_IMMINENT, value);
        }
      break_imminent_time[i] = value;
    }
}


void
AppletWindow::config_changed_notify(string key)
{
  (void) key;
  read_configuration();
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i] = 0;
    }
  reconfigure = true;
}
