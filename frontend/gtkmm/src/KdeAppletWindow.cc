// KdeAppletWindow.cc --- Applet info Window
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

#include "KdeAppletWindow.hh"
#include "AppletControl.hh"

#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "Menus.hh"
#include "System.hh"

#include "CoreInterface.hh"
#include "CoreFactory.hh"

#include <gdk/gdkcolor.h>

#include "KdeWorkraveControl.hh"

#ifdef HAVE_KDE
#include <dcopclient.h>
#include <kapp.h>
#include <kde_applet/kworkraveapplet_stub.h>
#endif


//! Constructor.
/*!
 *  \param control Interface to the controller.
 */
KdeAppletWindow::KdeAppletWindow(AppletControl *control) :
  plug(NULL),
  container(NULL),
  tray_menu(NULL),
  applet_vertical(false),
  applet_size(0),
  control(control),
  applet_active(false)
{
  KdeWorkraveControl::init();
  init_applet();
}


//! Destructor.
KdeAppletWindow::~KdeAppletWindow()
{
  delete plug;
  delete container;
}


//! Initializes the applet window.
void
KdeAppletWindow::init_applet()
{
  TRACE_ENTER("KdeAppletWindow::init");
  TRACE_EXIT();
}


//! Cleanup the applet window.
void
KdeAppletWindow::cleanup_applet()
{
}


//! Initializes the native kde applet.
AppletWindow::AppletActivateResult
KdeAppletWindow::activate_applet()
{
  TRACE_ENTER("KdeAppletWindow::activate_applet");

  if (applet_active)
    {
      TRACE_EXIT();
      return AppletWindow::APPLET_ACTIVATE_VISIBLE;
      
    }
  bool ok = true;

  ok = get_vertical(applet_vertical);

  if (ok)
    {
      ok = get_size(applet_size);
    }
  
  if (ok)
    {
      // Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      // frame->set_border_width(2);

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      eventbox->signal_button_press_event().connect(MEMBER_SLOT(*this, &KdeAppletWindow::on_button_press_event));
      container = eventbox;
      
      // container = frame;

      plug = new Gtk::Plug((unsigned int)0);
      plug->add(*container);

      plug->signal_embedded().connect(MEMBER_SLOT(*this, &KdeAppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &KdeAppletWindow::delete_event));

      // Gtkmm does not wrap this event....
      g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                       G_CALLBACK(KdeAppletWindow::destroy_event), this);
      
      view = manage(new TimerBoxGtkView());
      timer_box_view = view;
      
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      view->set_geometry(applet_vertical, applet_size);
      view->show_all();
      
      container->add(*view);
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

      plug_window(plug->get_id());

      // somehow, signal_embedded is never triggered...
      control->activated(AppletControl::APPLET_KDE);
      applet_active = true;
    }

  TRACE_EXIT();
  return ok;
}


//! Destroys the native gnome applet.
void
KdeAppletWindow::deactivate_applet()
{
  if (applet_active)
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
  applet_active = false;
  control->deactivated(AppletControl::APPLET_KDE);
}


//! Applet window is deleted. Destroy applet.
bool
KdeAppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  deactivate_applet();
  control->deactivated(AppletControl::APPLET_KDE);
  return true;
}
    

//! Fires up the applet (as requested by the native kde applet).
void
KdeAppletWindow::fire_kde_applet()
{
  control->show(AppletControl::APPLET_KDE);
}


//! Updates the applet window.
void
KdeAppletWindow::update()
{
  TRACE_ENTER("KdeAppletWindow::update");
  if (applet_active)
    {
      timer_box_control->update();

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
  
  TRACE_EXIT();
}

  
//! Destroy notification.
gboolean
KdeAppletWindow::destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  (void) widget;

  if (user_data != NULL)
    {
      KdeAppletWindow *applet = (KdeAppletWindow *) user_data;
      applet->delete_event(NULL);
    }
  return true;
}


//! User pressed some mouse button in the main window.
bool
KdeAppletWindow::on_button_press_event(GdkEventButton *event)
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
KdeAppletWindow::button_clicked(int button)
{
  (void) button;
  timer_box_control->force_cycle();
}


//! Notification of the system tray that the applet has been embedded.
void
KdeAppletWindow::on_embedded()
{
  TRACE_ENTER("KdeAppletWindow::on_embedded");
  if (applet_active)
    {
      container->set_size_request(-1,-1);
#ifdef HAVE_GTKMM24
      container->size_request(last_size);
#else
      container->size_request(&last_size);
#endif

      TRACE_MSG("Size = " << last_size.width << " " << last_size.height << " " << applet_vertical);
      view->set_geometry(applet_vertical, applet_size);

      TRACE_MSG(applet_size);
      KdeAppletWindow::set_size(last_size.width, last_size.height);
    }

  control->activated(AppletControl::APPLET_KDE);

  TRACE_EXIT();
}

bool
KdeAppletWindow::plug_window(int w)
{
  TRACE_ENTER("KdeAppletWindow::plug_window");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  dcop.embed_window(w);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}


bool
KdeAppletWindow::get_size(int &size)
{
  TRACE_ENTER("KdeAppletWindow::get_size");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  size = dcop.get_size();
  TRACE_MSG(dcop.ok() << " " << size);
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::get_vertical(bool &vertical)
{
  TRACE_ENTER("KdeAppletWindow::get_vertical");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  vertical = dcop.get_vertical();
  TRACE_MSG(dcop.ok() << " " << vertical);
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::set_size(int width, int height)
{
  TRACE_ENTER_MSG("KdeAppletWindow::set_size", width << " " << height);
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave"); 
  dcop.set_size(width, height);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}

