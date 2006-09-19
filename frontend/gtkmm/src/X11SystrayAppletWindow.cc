// X11SystrayAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#include <gtkmm/alignment.h>

#include "X11SystrayAppletWindow.hh"

#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "AppletControl.hh"
#include "Menus.hh"
#include "System.hh"

#include "eggtrayicon.h"

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
X11SystrayAppletWindow::X11SystrayAppletWindow(AppletControl *control) :
  view(NULL),
  plug(NULL),
  container(NULL),
  tray_menu(NULL),
  applet_vertical(false),
  applet_size(0),
  applet_active(false),
  control(control)
{
}


//! Destructor.
X11SystrayAppletWindow::~X11SystrayAppletWindow()
{
  delete plug;
  delete container;
}



//! Initializes the applet window.
void
X11SystrayAppletWindow::init_applet()
{
  TRACE_ENTER("X11SystrayAppletWindow::init_applet");
  TRACE_EXIT();
}
  
//! Cleanup the applet window.
void
X11SystrayAppletWindow::cleanup_applet()
{
}

//! Initializes the applet.
AppletWindow::AppletState
X11SystrayAppletWindow::activate_applet()
{
  TRACE_ENTER("X11SystrayAppletWindow::activate_applet");

  if (applet_active)
    {
      TRACE_EXIT();
      return APPLET_STATE_VISIBLE;
    }
  
  EggTrayIcon *tray_icon = egg_tray_icon_new("Workrave Tray Icon");
  AppletState ret =  APPLET_STATE_DISABLED;
  
  if (tray_icon != NULL)
    {
      plug = Glib::wrap(GTK_PLUG(tray_icon));
      
      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_visible_window(false);
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(MEMBER_SLOT(*this,
                                                                &X11SystrayAppletWindow::on_button_press_event));
      container = eventbox;
          
      view = manage(new TimerBoxGtkView());
      timer_box_view = view;
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      
      if (System::is_kde())
        {
          timer_box_control->set_force_empty(true);
        }
      
      container->add(*view);
      
      plug->signal_embedded().connect(MEMBER_SLOT(*this, &X11SystrayAppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &X11SystrayAppletWindow::delete_event));
      
      plug->add(*container);
      plug->show_all();
      
      // Tray menu
      Menus *menus = Menus::get_instance();
      menus->create_menu(Menus::MENU_APPLET);
      
      ret = AppletWindow::APPLET_STATE_VISIBLE;
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
      view->set_geometry(applet_vertical, 24);

      applet_active = true;
      ret = APPLET_STATE_VISIBLE;
    }
  
  TRACE_EXIT();
  return ret;
}


//! Destroys the applet.
void
X11SystrayAppletWindow::deactivate_applet()
{
  TRACE_ENTER("X11SystrayAppletWindow::destroy_applet");

  if (applet_active)
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
          container = NULL;
        }
      control->set_applet_state(AppletControl::APPLET_TRAY,
                                AppletWindow::APPLET_STATE_DISABLED);
    }
  
  applet_active = false;
  
  TRACE_EXIT();
}


//! Applet window is deleted. Destroy applet.
bool
X11SystrayAppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  deactivate_applet();
  control->set_applet_state(AppletControl::APPLET_TRAY,
                            AppletWindow::APPLET_STATE_DISABLED);
  return true;
}


//! Notification of the system tray that the applet has been embedded.
void
X11SystrayAppletWindow::on_embedded()
{
  TRACE_ENTER("X11SystrayAppletWindow::on_embedded");

  if (applet_active)
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
      view->set_geometry(applet_vertical, applet_size);
  
      TRACE_MSG(applet_size);
    }

  control->set_applet_state(AppletControl::APPLET_TRAY,
                            AppletWindow::APPLET_STATE_VISIBLE);

  TRACE_EXIT();
}

//! User pressed some mouse button in the main window.
bool
X11SystrayAppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

  if (applet_active &&
      event->type == GDK_BUTTON_PRESS)
    {
      if (event->button == 3 && tray_menu != NULL)
        {
          Menus::get_instance()->popup(Menus::MENU_APPLET,
                                       event->button, event->time);
          ret = true;
        }
      if (event->button == 1)
        {
          button_clicked(1);
          ret = true;
        }
    }
      
  return ret;
}

//! User clicked left mouse button.
void
X11SystrayAppletWindow::button_clicked(int button)
{
  (void) button;
  timer_box_control->force_cycle();
}
