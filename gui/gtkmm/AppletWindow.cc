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
// TODO: release CORBA memory.

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "AppletWindow.hh"
#include "TimerBox.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Configurator.hh"
#include "Menus.hh"
#include "Util.hh"

#ifdef HAVE_GNOME
#include "RemoteControl.hh"
#endif
#include "eggtrayicon.h"


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow() :
  mode(APPLET_DISABLED),
  plug(NULL),
  container(NULL),
  tray_menu(NULL),
#ifdef HAVE_GNOME
  applet_control(NULL),
#endif
  retry_init(false),
  reconfigure(false),
  applet_vertical(false),
  applet_size(0),
  applet_enabled(true)
{
  
#ifdef HAVE_GNOME
  Menus *menus = Menus::get_instance();
  menus->set_applet_window(this);
#endif

  init();
}


//! Destructor.
AppletWindow::~AppletWindow()
{
  delete plug;
  delete container;
}



//! Initializes the main window.
void
AppletWindow::init()
{
  TRACE_ENTER("AppletWindow::init");

  Configurator *config = GUIControl::get_instance()->get_configurator();
  config->add_listener(TimerBox::CFG_KEY_TIMERBOX + "applet", this);

  read_configuration();
  
  // Create the applet.
  if (applet_enabled)
    {
      init_applet();
    }
  
  TRACE_EXIT();
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
      plug = Glib::wrap(GTK_PLUG(tray_icon));

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(SigC::slot(*this, &AppletWindow::on_button_press_event));
      container = eventbox;

      timers_box = new TimerBox("applet");
      container->add(*timers_box);
      
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

      timers_box->set_geometry(applet_vertical, 24);
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
  // FIXME: leak
  if (applet_control == NULL)
    {
      applet_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_AppletControl",
                                                          Bonobo_ACTIVATION_FLAG_EXISTING_ONLY, NULL, &ev);
    }
  
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

      timers_box = new TimerBox("applet");
      timers_box->set_geometry(applet_vertical, applet_size);
      timers_box->show_all();
      
      container->add(*timers_box);
      container->show_all();
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
  destroy_applet();
  return true;
}
    

//! Fire up the applet (as requested by the native gnome applet).
void
AppletWindow::fire()
{
  if (mode == APPLET_TRAY)
    {
      destroy_tray_applet();
    }
  
  if (mode == APPLET_DISABLED && applet_enabled)
    {
      retry_init = true;
    }
}

#ifdef HAVE_GNOME
//! Fire up the applet (as requested by the native gnome applet).
void
AppletWindow::set_applet_control(GNOME_Workrave_AppletControl applet_control)
{
  if (mode == APPLET_TRAY)
    {
      destroy_tray_applet();
    }

  if (this->applet_control != NULL)
    {
      // FIXME: free old interface
    }
  
  this->applet_control = applet_control;
  
  if (mode == APPLET_DISABLED && applet_enabled)
    {
      retry_init = true;
    }
}
#endif

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
          timers_box->update();
        }
    }
}



#ifdef HAVE_GNOME
//! Sets the state of a toggle menu item.
void
AppletWindow::set_menu_active(int menu, bool active)
{
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
}


//! Retrieves the state of a toggle menu item.
bool
AppletWindow::get_menu_active(int menu)
{
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
  return ret;
}


//! Sets the orientation of the applet.
void
AppletWindow::set_applet_vertical(bool v)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_vertical", applet_vertical);

  applet_vertical = v;
  reconfigure = true;

  if (timers_box != NULL)
    {
      timers_box->set_geometry(applet_vertical, applet_size);
    }
  
  TRACE_EXIT();
}


//! Sets the size of the applet.
void
AppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_size", size);

  applet_size = size;
  reconfigure = true;

  if (timers_box != NULL)
    {
      timers_box->set_geometry(applet_vertical, applet_size);
    }
  
  TRACE_EXIT();
}
#endif
  

//! Users pressed some mouse button in the main window.
bool
AppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

  if (event->type == GDK_BUTTON_PRESS)
    {
      if (event->button == 3 && tray_menu != NULL)
        {
          tray_menu->popup(event->button, event->time);
          ret = true;
        }
      if (event->button == 1) // FIXME:  && visible_count == 0)
        {
          button_clicked(1);
          ret = true;
        }
    }
  
  return ret;
}


void
AppletWindow::button_clicked(int button)
{
  (void) button;
  
  GUI *gui = GUI::get_instance();
  assert(gui != NULL);
  
  gui->toggle_main_window();
}


AppletWindow::AppletMode
AppletWindow::get_applet_mode() const
{
  return mode;
}


//! Reads the applet configuration.
void
AppletWindow::read_configuration()
{
  applet_enabled = TimerBox::is_enabled("applet");
}


//! Callback that the configuration has changed.
void
AppletWindow::config_changed_notify(string key)
{
  (void) key;

  read_configuration();
  reconfigure = true;
}
