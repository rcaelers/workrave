// AppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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
// TODO: refactor. split into 4 classes.

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "AppletWindow.hh"
#include "MainWindow.hh"
#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"

#ifdef HAVE_GNOME
#include "RemoteControl.hh"
#endif
#include "eggtrayicon.h"

#ifdef HAVE_KDE
#include "KdeAppletWindow.hh"
#endif

#include "ConfiguratorInterface.hh"
#include "CoreInterface.hh"
#include "CoreFactory.hh"


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
AppletWindow::AppletWindow() :
  timer_box_view(NULL),
  timer_box_control(NULL),
  mode(APPLET_DISABLED),
  plug(NULL),
  container(NULL),
  tray_menu(NULL),
#ifdef HAVE_GNOME
  applet_control(NULL),
#endif
  retry_init(false),
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
  set_mainwindow_applet_active(false);
}



//! Initializes the applet window.
void
AppletWindow::init()
{
  TRACE_ENTER("AppletWindow::init");

  // Read configuration and start monitoring it.
  read_configuration();
  ConfiguratorInterface *config = CoreFactory::get_configurator();
  config->add_listener(TimerBoxControl::CFG_KEY_TIMERBOX + "applet", this);
  
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
#ifdef HAVE_KDE
  if (init_kde_applet())
    {
      mode = APPLET_KDE;
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
#ifdef HAVE_KDE
  else if (mode == APPLET_KDE)
    {
      destroy_kde_applet();
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
  
  set_mainwindow_applet_active(false);
  EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
      
  if (tray_icon != NULL)
    {
      plug = Glib::wrap(GTK_PLUG(tray_icon));

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(MEMBER_SLOT(*this, &AppletWindow::on_button_press_event));
      container = eventbox;

      timer_box_view = manage(new TimerBoxGtkView());
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      container->add(*timer_box_view);

      plug->signal_embedded().connect(MEMBER_SLOT(*this, &AppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &AppletWindow::delete_event));
      
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
#ifdef HAVE_GTKMM24
      Gtk::Requisition req;
      plug->size_request(req);
      applet_size = req.height;
#else
      GtkRequisition req;
      plug->size_request(&req);
      applet_size = req.height;
#endif      
      timer_box_view->set_geometry(applet_vertical, 24);

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
      set_mainwindow_applet_active(false);
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
      // Initialize applet GUI.
      
      Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      frame->set_border_width(2);

      container = frame;

      plug = new Gtk::Plug(id);
      plug->add(*frame);

      plug->signal_embedded().connect(MEMBER_SLOT(*this, &AppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &AppletWindow::delete_event));

      // Gtkmm does not wrap this event....
      g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                       G_CALLBACK(AppletWindow::destroy_event), this);
      
      timer_box_view = manage(new TimerBoxGtkView());
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      timer_box_view->set_geometry(applet_vertical, applet_size);
      timer_box_view->show_all();
      
      container->add(*timer_box_view);
      container->show_all();
      plug->show_all();
      
      Menus *menus = Menus::get_instance();
      if (menus != NULL)
        {
          menus->resync_applet();
        }

#ifndef HAVE_EXERCISES
      GNOME_Workrave_AppletControl_set_menu_active(applet_control, "/commands/Exercises", false, &ev);
#endif
#ifndef HAVE_DISTRIBUTION
      GNOME_Workrave_AppletControl_set_menu_active(applet_control, "/commands/Network", false, &ev);
#endif

      // somehow, signal_embedded is never triggered...
      set_mainwindow_applet_active(true);
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


#ifdef HAVE_KDE
//! Initializes the native kde applet.
bool
AppletWindow::init_kde_applet()
{
  TRACE_ENTER("AppletWindow::init_kde_applet");

  bool ok = true;

  ok = KdeAppletWindow::get_vertical(applet_vertical);

  if (ok)
    {
      ok = KdeAppletWindow::get_size(applet_size);
    }
  
  if (ok)
    {
      // Initialize applet GUI.
      
      // Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      // frame->set_border_width(2);

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      eventbox->signal_button_press_event().connect(MEMBER_SLOT(*this, &AppletWindow::on_button_press_event));
      container = eventbox;
      
      // container = frame;

      plug = new Gtk::Plug((unsigned int)0);
      plug->add(*container);

      plug->signal_embedded().connect(MEMBER_SLOT(*this, &AppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &AppletWindow::delete_event));

      // Gtkmm does not wrap this event....
      g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                       G_CALLBACK(AppletWindow::destroy_event), this);
      
      timer_box_view = manage(new TimerBoxGtkView());
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      timer_box_view->set_geometry(applet_vertical, applet_size);
      timer_box_view->show_all();
      
      container->add(*timer_box_view);
      container->show_all();
      plug->show_all();

      Gtk::Requisition req;
      container->size_request(req);
      TRACE_MSG("Size = " << req.width << " " << req.height << " " << applet_vertical);
      
      // Tray menu
      if (tray_menu == NULL)
        {
          Menus *menus = Menus::get_instance();
          tray_menu = menus->create_tray_menu();
        }

      KdeAppletWindow::plug_window(plug->get_id());

      // somehow, signal_embedded is never triggered...
      set_mainwindow_applet_active(true);
    }

  TRACE_EXIT();
  return ok;
}


//! Destroys the native gnome applet.
void
AppletWindow::destroy_kde_applet()
{
  if (mode == APPLET_KDE)
    {
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
    }
  mode = APPLET_DISABLED;
}
#endif


//! Applet window is deleted. Destroy applet.
bool
AppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  set_mainwindow_applet_active(false);
  destroy_applet();
  return true;
}
    

//! Fires up the applet (as requested by the native gnome applet).
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
//! Sets the applet control callback interface.
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


//! Updates the applet window.
void
AppletWindow::update()
{
  TRACE_ENTER("AppletWindow::update");
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
          timer_box_control->update();

#ifdef HAVE_KDE
          if (mode == APPLET_KDE)
            {
#ifdef HAVE_GTKMM24
              Gtk::Requisition req;
              container->size_request(req);
#else
              GtkRequisition req;
              container->size_request(&req);
#endif

              TRACE_MSG("Size = " << req.width << " " << req.height << " " << applet_vertical);
              if (req.width != last_size.width || req.height != last_size.height)
                {
                  last_size = req;
                  KdeAppletWindow::set_size(last_size.width, last_size.height);
                }
            }
#endif              
        }
    }
  TRACE_EXIT();
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

  if (timer_box_view != NULL)
    {
      timer_box_view->set_geometry(applet_vertical, applet_size);
    }
  
  TRACE_EXIT();
}


//! Sets the size of the applet.
void
AppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_size", size);

  plug->queue_resize();
  
  applet_size = size;

  if (timer_box_view != NULL)
    {
      timer_box_view->set_geometry(applet_vertical, applet_size);
    }
  
  TRACE_EXIT();
}


//! Destroy notification.
gboolean
AppletWindow::destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  (void) widget;
  if (user_data != NULL)
    {
      AppletWindow *applet = (AppletWindow *) user_data;
      applet->delete_event(NULL);
    }
  return true;
}
#endif
  

//! User pressed some mouse button in the main window.
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


//! User clicked left mouse button.
void
AppletWindow::button_clicked(int button)
{
  (void) button;
  
  GUI *gui = GUI::get_instance();
  assert(gui != NULL);
  
  gui->toggle_main_window();
}


//! Returns the applet mode.
AppletWindow::AppletMode
AppletWindow::get_applet_mode() const
{
  return mode;
}


//! Reads the applet configuration.
void
AppletWindow::read_configuration()
{
  applet_enabled = TimerBoxControl::is_enabled("applet");
}


//! Callback that the configuration has changed.
void
AppletWindow::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("AppletWindow::config_changed_notify", key);
  (void) key;
  read_configuration();
  TRACE_EXIT();
}


//! Notification of the system tray that the applet has been embedded.
void
AppletWindow::on_embedded()
{
  TRACE_ENTER("AppletWindow::on_embedded");
  set_mainwindow_applet_active(true);

  if (mode == APPLET_TRAY)
    {
#ifdef HAVE_GTKMM24
      Gtk::Requisition req;
      plug->size_request(req);
      applet_size = req.height;
#else
      GtkRequisition req;
      plug->size_request(&req);
      applet_size = req.height;
#endif

      TRACE_MSG("Size = " << req.width << " " << req.height << " " << applet_vertical);
      timer_box_view->set_geometry(applet_vertical, applet_size);

      TRACE_MSG(applet_size);
    }
#ifdef HAVE_KDE
  else if (mode == APPLET_KDE)
    {
      container->set_size_request(-1,-1);
#ifdef HAVE_GTKMM24
      container->size_request(last_size);
#else
      container->size_request(&last_size);
#endif

      TRACE_MSG("Size = " << last_size.width << " " << last_size.height << " " << applet_vertical);
      timer_box_view->set_geometry(applet_vertical, applet_size);

      TRACE_MSG(applet_size);
      if (mode == APPLET_KDE)
        {
          KdeAppletWindow::set_size(last_size.width, last_size.height);
        }
    }
#endif      
  TRACE_EXIT();
}


//! Sets the applet active state.
void
AppletWindow::set_mainwindow_applet_active(bool a)
{
  TRACE_ENTER_MSG("AppletWindow::set_mainwindow_applet_active", a);
  GUI *gui = GUI::get_instance();
  MainWindow *main = gui->get_main_window();
  if (main != NULL)
    {
      main->set_applet_active(a);
    }
  TRACE_EXIT();
}
