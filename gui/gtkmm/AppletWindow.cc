// AppletWindow.cc --- Applet info Window
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
//
// TODO: split system tray and gnome applet.
// TODO: release CORBA memory.

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "AppletWindow.hh"
#include "Configurator.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Menus.hh"
#include "TimeBar.hh"
#include "TimerInterface.hh"

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
AppletWindow::AppletWindow() :
  mode(APPLET_DISABLED),
  retry_init(false),
  reconfigure(false),
  plug(NULL),
  container(NULL),
  timers_box(NULL),
  tray_menu(NULL),
#ifdef HAVE_GNOME
  applet_control(NULL),
#endif
  cycle_time(10),
  applet_vertical(false),
  applet_size(0),
  applet_enabled(true)
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_position[i] = i;
      break_flags[i] = 0;
      break_imminent_time[i] = 0;
      current_content[i] = -1;
      
      for (int j = 0; j < GUIControl::BREAK_ID_SIZEOF; j++)
        {
          break_slots[i][j] = -1;
        }
      break_slot_cycle[i] = 0;
    }
  
#ifdef HAVE_GNOME
  Menus *menus = Menus::get_instance();
  menus->set_applet_window(this);
#endif
  
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
  if (plug != NULL)
    {
      delete plug;
    }
  if (container != NULL)
    {
      delete container;
    }

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (timer_names[i] != NULL)
        timer_names[i]->unreference();
      if (timer_times[i] != NULL)
        timer_times[i]->unreference();
    }
  
  TRACE_EXIT();
}



//! Initializes the main window.
void
AppletWindow::init()
{
  TRACE_ENTER("AppletWindow::init");

  // Load the configuration
  read_configuration();

  // Listen for configugration changes.
  Configurator *config = GUIControl::get_instance()->get_configurator();
  config->add_listener(AppletWindow::CFG_KEY_APPLET, this);

  init_widgets();

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      timer_names[i]->reference();
      timer_times[i]->reference();
    }
  
  // Create the applet.
  if (applet_enabled)
    {
      init_applet();

      if (mode != APPLET_DISABLED)
        {
          init_table();
        }
    }
  
  TRACE_EXIT();
}
  

//! Initializes the applet.
void
AppletWindow::init_table()
{
  TRACE_ENTER("AppletWindow::init_table");

  // Determine what breaks to show.
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      init_slot(i);
    }

  
  // Compute number of vivisble breaks.
  int number_of_timers = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (break_slots[i][0] != -1)
        {
          number_of_timers++;
        }
    }

  
  // Compute table dimensions.
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
      rows = applet_size / size.height;

      if (rows <= 0)
        {
          rows = 1;
        }
      columns = (number_of_timers + rows - 1) / rows;
      
      plug->set_size_request(-1, applet_size);
    }

  
  // Create table
  if (timers_box == NULL)
    {
      timers_box = new Gtk::Table(rows, 2 * columns, false);
      timers_box->set_spacings(2);
      timers_box->reference();
      container->add(*timers_box);
    }

  
  // Compute new content.
  int new_content[GUIControl::BREAK_ID_SIZEOF];
  int slot = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {

      new_content[i] = -1;
      int cycle = break_slot_cycle[i];
      int id = break_slots[i][cycle]; // break id
      if (id != -1)
        {
          new_content[slot] = id;
          TRACE_MSG(slot << " " << id);

          slot++;
        }
    }


  // Remove old
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && id != new_content[i])
        {
          TRACE_MSG("remove " << id);
          Gtk::Widget *child = timer_names[id];
          timers_box->remove(*child);
          child = timer_times[id];
          timers_box->remove(*child);
        }
    }
  
  // Fill table.
  for (int i = 0; i < slot; i++)
    {
      int id = new_content[i];
      int cid = current_content[i];

      TRACE_MSG(i << " " << id << " " << cid);
      if (id != cid)
        {
          current_content[i] = id;
          
          int cur_row = i % rows;
          int cur_col = i / rows;
          
          if (!applet_vertical && applet_size != -1)
            {
              timer_times[id]->set_size_request(-1, applet_size / rows - 1 * (rows + 1) - 2);
            }
          
          timers_box->attach(*timer_names[id], 2 * cur_col, 2 * cur_col + 1, cur_row, cur_row + 1,
                             Gtk::FILL, Gtk::SHRINK);
          timers_box->attach(*timer_times[id], 2 * cur_col + 1, 2 * cur_col + 2, cur_row, cur_row + 1,
                             Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);

        }
    }

  container->show_all();
  plug->show_all();
  TRACE_EXIT();
}


//! Compute what break to show on the specified location.
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


//! Cycles through the breaks.
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


//! Initializes the applet.
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


//! Destroys the applet.
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


//! Initializes the system tray applet.
bool
AppletWindow::init_tray_applet()
{
  TRACE_ENTER("AppletWindow::init_tray_applet");
  bool ret = false;
  
  EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
  if (tray_icon != NULL)
    {
      Gtk::EventBox *eventbox = new Gtk::EventBox;
        
      // Necessary for popup menu 
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(SigC::slot(*this, &AppletWindow::on_button_press_event));
      container = eventbox;

      if (timers_box != NULL)
        {
          container->add(*timers_box);
        }
      
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
    }

  TRACE_EXIT();
  return ret;
}


//! Destroys the system tray applet.
void
AppletWindow::destroy_tray_applet()
{
  if (mode == APPLET_TRAY)
    {
      if (plug != NULL)
        {
          plug->remove();
          delete plug;
          plug = NULL;
        }
      if (container != NULL)
        {
          container->remove();
          delete container;
        }
    }
  mode = APPLET_DISABLED;
}


#ifdef HAVE_GNOME
//! Initializes the native gnome applet.
bool
AppletWindow::init_gnome_applet()
{
  TRACE_ENTER("AppletWindow::init_gnome_applet");
  bool ok = true;

  // Initialize bonobo activation.
  bonobo_activate();

  CORBA_Environment ev;
  CORBA_exception_init (&ev);

  // Connect to the applet.
  applet_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_AppletControl",
                                                      Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, NULL, &ev);

  // Socket ID of the applet.
  long id = 0;
  if (applet_control != NULL && !BONOBO_EX(&ev))
    {
      id = GNOME_Workrave_AppletControl_get_socket_id(applet_control, &ev);
      ok = !BONOBO_EX(&ev);
    }

  if (ok)
    {
      // Retrieve applet size.
      applet_size = GNOME_Workrave_AppletControl_get_size(applet_control, &ev);
      ok = !BONOBO_EX(&ev);
    }

  if (ok)
    {
      // Retrieve applet orientation.
      applet_vertical =  GNOME_Workrave_AppletControl_get_vertical(applet_control, &ev);
      ok = !BONOBO_EX(&ev);
    }

  if (ok)
    {
      // Register at applet.
      RemoteControl *remote = RemoteControl::get_instance();
      if (remote != NULL)
        {
          WorkraveControl *control = remote->get_remote_control();

          if (control != NULL)
            {
              Bonobo_Unknown x = bonobo_object_corba_objref(BONOBO_OBJECT(control));
              GNOME_Workrave_AppletControl_register_control(applet_control, x, &ev);
            }
        }
    }
  
  if (ok)
    {
      // Initialize applet GUI.
      
      Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      frame->set_border_width(2);
      container = frame;

      plug = new Gtk::Plug(id);
      plug->add(*frame);

      if (timers_box != NULL)
        {
          container->add(*timers_box);
          container->show_all();
        }

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


//! Destroys the native gnome applet.
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
          if (control != NULL)
            {
              // Unregister workrave.
              Bonobo_Unknown x = bonobo_object_corba_objref(BONOBO_OBJECT(control));
              GNOME_Workrave_AppletControl_unregister_control(applet_control, x, &ev);
            }
          
          CORBA_exception_free(&ev);
        }

      // Cleanup Widgets.
      if (plug != NULL)
        {
          plug->remove();
          delete plug;
          plug = NULL;
        }
      
      if (container != NULL)
        {
          container->remove();
          delete container;
          container = NULL;
        }
      applet_control = NULL; // FIXME: free memory.
    }
  mode = APPLET_DISABLED;
}
#endif


//! Applet window is deleted. Destroy applet.
bool
AppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  TRACE_ENTER("AppletWindow::deleted");
  destroy_applet();
  TRACE_EXIT();
  return true;
}
    

//! Fire up the applet (as requested by the native gnome applet).
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
      // Applet is disabled.
      
      if (applet_enabled)
        {
          // Enable applet.
          retry_init = true;
        }

      if (retry_init)
        {
          // Attempt to initialize the applet again.
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
      // Applet is enabled.
      if (!applet_enabled)
        {
          // Disable applet.
          destroy_applet();
        }
      else
        {
          // Cycle through the timers.
          time_t t = time(NULL);
          if (t % cycle_time == 0)
            {
              init_table();
              cycle_slots();
            }

          // Configuration was changed. reinit.
          if (reconfigure)
            {
              init_table();
              reconfigure = false;
            }

          // Update the timer widgets.
          update_widgets();
        }
    }
}



#ifdef HAVE_GNOME
//! Sets the state of a toggle menu item.
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


//! Retrieves the state of a toggle menu item.
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


//! Sets the orientation of the applet.
void
AppletWindow::set_applet_vertical(bool v)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_vertical", applet_vertical);

  applet_vertical = v;
  reconfigure = true;

  TRACE_EXIT();
}


//! Sets the size of the applet.
void
AppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_size", size);

  applet_size = size;
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


//! Reads the applet configuration.
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


//! Callback that the configuration has changed.
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
