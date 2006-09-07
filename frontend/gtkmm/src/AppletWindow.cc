// AppletWindow.cc --- Applet info Window
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
// TODO: release CORBA memory.
// TODO: refactor. split into 4 classes.

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <gtkmm/alignment.h>

#include "AppletWindow.hh"
#include "MainWindow.hh"
#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "System.hh"

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

#include <gdk/gdkcolor.h>

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
#ifdef HAVE_KDE
  KdeAppletWindow::init();
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
      eventbox->set_visible_window(false);
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(MEMBER_SLOT(*this, &AppletWindow::on_button_press_event));
      container = eventbox;

      timer_box_view = manage(new TimerBoxGtkView());
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);

      if (System::is_kde())
        {
          timer_box_control->set_force_empty(true);
        }
      
      container->add(*timer_box_view);

      plug->signal_embedded().connect(MEMBER_SLOT(*this, &AppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &AppletWindow::delete_event));
      
      plug->add(*container);
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

      plug->set_events(plug->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      
      plug->signal_embedded().connect(MEMBER_SLOT(*this, &AppletWindow::on_embedded));
      plug->signal_delete_event().connect(MEMBER_SLOT(*this, &AppletWindow::delete_event));

      // Gtkmm does not wrap this event....
      g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                       G_CALLBACK(AppletWindow::destroy_event), this);
      
      timer_box_view = manage(new TimerBoxGtkView());
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      timer_box_view->set_geometry(applet_vertical, applet_size);
      timer_box_view->show_all();

      plug->signal_button_press_event().connect(MEMBER_SLOT(*this, &AppletWindow::on_button_press_event));
      plug->signal_button_release_event().connect(MEMBER_SLOT(*this, &AppletWindow::on_button_press_event));
      
      container->add(*timer_box_view);
      container->show_all();
      plug->show_all();

      Menus *menus = Menus::get_instance();

      // Tray menu
      if (tray_menu == NULL)
        {
          tray_menu = menus->create_tray_menu();
        }
      
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
AppletWindow::fire_gnome_applet()
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


//! Fires up the applet (as requested by the native kde applet).
void
AppletWindow::fire_kde_applet()
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


//! Sets the size of the applet.
void
AppletWindow::set_applet_background(int type, GdkColor &color, long xid)
{
  TRACE_ENTER_MSG("AppletWindow::set_applet_pixmap", xid);

  if (plug == NULL)
    {
      return;
    }

  // FIXME: convert to Gtkmm and check for memory leaks.
  GtkWidget *widget = GTK_WIDGET(plug->gobj());
  GdkPixmap *pixmap = NULL;
  
  if (type == 2)
    {
      int width, height;
      
      gdk_error_trap_push();
      GdkPixmap *orig_pixmap = gdk_pixmap_foreign_new(xid);

      if (orig_pixmap != NULL)
        {
          gdk_drawable_get_size(GDK_DRAWABLE(orig_pixmap), &width, &height);

          GdkPixbuf *pbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
          GdkWindow *rootwin = gdk_get_default_root_window ();
          GdkColormap *cmap = gdk_drawable_get_colormap(GDK_DRAWABLE(rootwin));
                
          gdk_pixbuf_get_from_drawable (pbuf, orig_pixmap, cmap, 0, 0,
                                        0, 0, width , height);

          /* put background onto the widget */
          gdk_pixbuf_render_pixmap_and_mask (pbuf, &pixmap, NULL, 127);

          gdk_flush();
          gdk_error_trap_pop();

          if (pixmap != NULL)
            {
              gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap),
                                        gdk_drawable_get_colormap(
                                                                  gdk_get_default_root_window()));
              
            }

          g_object_unref(G_OBJECT(orig_pixmap));
          g_object_unref(G_OBJECT(pbuf));
        }
    }
  
  GtkRcStyle *rc_style = gtk_rc_style_new();
  GtkStyle *style = NULL;
  
  gtk_widget_set_style (widget, NULL);
  gtk_widget_modify_style (widget, rc_style);
  
  switch (type)
    {
    case 0: //PANEL_NO_BACKGROUND:
      break;
    case 1: //PANEL_COLOR_BACKGROUND:
      gtk_widget_modify_bg (widget,
                            GTK_STATE_NORMAL, &color);
      break;
    case 2: //PANEL_PIXMAP_BACKGROUND:
      style = gtk_style_copy (widget->style);
      if (style->bg_pixmap[GTK_STATE_NORMAL])
        g_object_unref (style->bg_pixmap[GTK_STATE_NORMAL]);
      style->bg_pixmap[GTK_STATE_NORMAL] = (GdkPixmap *)g_object_ref (pixmap);
      gtk_widget_set_style (widget, style);
      g_object_unref (style);
      break;
    }

  gtk_rc_style_unref (rc_style);

  if (pixmap != NULL)
    {
      g_object_unref(G_OBJECT(pixmap));
    }
  
  TRACE_EXIT();
}
#endif
  
#if defined(HAVE_GNOME) || defined(HAVE_KDE)
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

  if (mode != APPLET_GNOME)
    {
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
    }
  else
    {
      /* Taken from:
       *
       * bonobo-plug.c: a Gtk plug wrapper.
       *
       * Author:
       *   Martin Baulig     (martin@home-of-linux.org)
       *   Michael Meeks     (michael@ximian.com)
       *
       * Copyright 2001, Ximian, Inc.
       *                 Martin Baulig.
       */
      
      XEvent xevent;
      GtkWidget *widget = GTK_WIDGET(plug->gobj());
      bool ok = false;
      
      if (event->type == GDK_BUTTON_PRESS)
        {
          xevent.xbutton.type = ButtonPress;

          /* X does an automatic pointer grab on button press
           * if we have both button press and release events
           * selected.
           * We don't want to hog the pointer on our parent.
           */
          gdk_display_pointer_ungrab(gtk_widget_get_display (widget),
                                     GDK_CURRENT_TIME);
          ok = true;
        }
      else if (event->type == GDK_BUTTON_RELEASE)
        {
          xevent.xbutton.type = ButtonRelease;
          ok = true;
        }

      if (ok)
        {
          xevent.xbutton.display     = GDK_WINDOW_XDISPLAY(widget->window);
          xevent.xbutton.window      = GDK_WINDOW_XWINDOW(GTK_PLUG(widget)->socket_window);
          xevent.xbutton.root        = GDK_WINDOW_XWINDOW(gdk_screen_get_root_window
                                                          (gdk_drawable_get_screen(widget->window)));
          /*
           * FIXME: the following might cause
           *        big problems for non-GTK apps
           */
          xevent.xbutton.x           = 0;
          xevent.xbutton.y           = 0;
          xevent.xbutton.x_root      = 0;
          xevent.xbutton.y_root      = 0;
          xevent.xbutton.state       = event->state;
          xevent.xbutton.button      = event->button;
          xevent.xbutton.same_screen = TRUE; /* FIXME ? */

          xevent.xbutton.serial      = 0;
          xevent.xbutton.send_event  = TRUE;
          xevent.xbutton.subwindow   = 0;
          xevent.xbutton.time        = event->time;

          gdk_error_trap_push();
          
          XSendEvent(GDK_WINDOW_XDISPLAY(widget->window),
                     GDK_WINDOW_XWINDOW(GTK_PLUG(widget)->socket_window),
                     False, NoEventMask, &xevent);
          
          gdk_flush();
          gdk_error_trap_pop();
        }
    }
      
  return ret;
}


//! User clicked left mouse button.
void
AppletWindow::button_clicked(int button)
{
  (void) button;
  
  //   GUI *gui = GUI::get_instance();
  //   assert(gui != NULL);
  //   gui->toggle_main_window();

  timer_box_control->force_cycle();
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
