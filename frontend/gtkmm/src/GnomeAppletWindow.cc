// GnomeAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <gtkmm/alignment.h>

#include "GnomeAppletWindow.hh"

#include "AppletControl.hh"
#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "System.hh"

#include "RemoteControl.hh"

#include "ICore.hh"
#include "CoreFactory.hh"

#include <gdk/gdkcolor.h>
#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xlib.h>

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
GnomeAppletWindow::GnomeAppletWindow(AppletControl *control) :
  view(NULL),
  plug(NULL),
  container(NULL),
  applet_control(NULL),
  applet_orientation(ORIENTATION_UP),
  applet_size(0),
  control(control),
  applet_active(false)
{
}


//! Destructor.
GnomeAppletWindow::~GnomeAppletWindow()
{
  delete plug;
  delete container;
  delete timer_box_control;
  delete timer_box_view;
}


//! Initializes the native gnome applet.
AppletWindow::AppletState
GnomeAppletWindow::activate_applet()
{
  TRACE_ENTER("GnomeAppletWindow::activate_applet");
  bool ok = true;

  if (!applet_active)
    {
      // Initialize bonobo activation.
      bonobo_activate();

      CORBA_Environment ev;
      CORBA_exception_init (&ev);

      // Connect to the applet.
      // FIXME: leak
      if (applet_control == NULL)
        {
          applet_control = bonobo_activation_activate_from_id("OAFIID:GNOME_Workrave_AppletControl",
                                                              Bonobo_ACTIVATION_FLAG_EXISTING_ONLY,
                                                              NULL, &ev);
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
          applet_orientation =  (Orientation)
            GNOME_Workrave_AppletControl_get_orientation(applet_control, &ev);
          ok = !BONOBO_EX(&ev);
        }


      if (ok)
        {
          // Initialize applet GUI.

          Gtk::Alignment *frame = new Gtk::Alignment(0.0, 0.0, 0.0, 0.0);
          frame->set_border_width(0);

          container = frame;

          plug = new Gtk::Plug(id);
          plug->add(*frame);

          plug->set_events(plug->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

          plug->signal_embedded().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_embedded));
          plug->signal_delete_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::delete_event));

          // Gtkmm does not wrap this event....
          g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                           G_CALLBACK(GnomeAppletWindow::destroy_event), this);

          view = new TimerBoxGtkView();
          timer_box_view = view;
          timer_box_control = new TimerBoxControl("applet", *timer_box_view);

          view->set_geometry(applet_orientation, applet_size);
          view->show_all();

          plug->signal_button_press_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_button_press_event));
          plug->signal_button_release_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_button_press_event));

          container->add(*view);
          container->show_all();
          plug->show_all();

#ifndef HAVE_EXERCISES
          GNOME_Workrave_AppletControl_set_menu_active(applet_control, "/commands/Exercises", false, &ev);
#endif
#ifndef HAVE_DISTRIBUTION
          GNOME_Workrave_AppletControl_set_menu_active(applet_control, "/commands/Network", false, &ev);
#endif
        }

      if (!ok)
        {
          applet_control = NULL;
        }

      Menus *menus = Menus::get_instance();
      menus->resync();
      
      CORBA_exception_free(&ev);
    }

  if (ok)
    {
      applet_active = true;
    }

  TRACE_EXIT();
  return ok ?
    AppletWindow::APPLET_STATE_VISIBLE :
    AppletWindow::APPLET_STATE_DISABLED;
}


//! Destroys the native gnome applet.
void
GnomeAppletWindow::deactivate_applet()
{
  TRACE_ENTER("GnomeAppletWindow::deactivate_applet");
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

      delete timer_box_control;
      timer_box_control = NULL;

      delete timer_box_view;
      timer_box_view = NULL;
      view = NULL;

      applet_control = NULL; // FIXME: free memory.
    }

  applet_active = false;
  TRACE_EXIT();
}


//! Applet window is deleted. Destroy applet.
bool
GnomeAppletWindow::delete_event(GdkEventAny *event)
{
  TRACE_ENTER("GnomeAppletWindow::delete_event");
  (void) event;
  deactivate_applet();
  control->set_applet_state(AppletControl::APPLET_GNOME,
                            AppletWindow::APPLET_STATE_DISABLED);
  TRACE_EXIT();
  return true;
}


//! Fires up the applet (as requested by the native gnome applet).
void
GnomeAppletWindow::fire_gnome_applet()
{
  control->show(AppletControl::APPLET_GNOME);
}


//! Sets the applet control callback interface.
void
GnomeAppletWindow::set_applet_control(GNOME_Workrave_AppletControl applet_control)
{
  TRACE_ENTER("GnomeAppletWindow::set_applet_control");
  if (this->applet_control != NULL)
    {
      // FIXME: free old interface
    }

  this->applet_control = applet_control;

  control->show(AppletControl::APPLET_GNOME);
  TRACE_EXIT();
}


//! Sets the state of a toggle menu item.
void
GnomeAppletWindow::set_menu_active(int menu, bool active)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_menu_active", menu << " " << active);
  CORBA_Environment ev;

  if (applet_control != NULL)
    {
      CORBA_exception_init (&ev);
      switch (menu)
        {
        case MENUSYNC_MODE_NORMAL:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Normal",
                                                       active, &ev);
          break;
        case MENUSYNC_MODE_SUSPENDED:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Suspended",
                                                       active, &ev);
          break;
        case MENUSYNC_MODE_QUIET:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/Quiet",
                                                       active, &ev);
          break;
        case MENUSYNC_SHOW_LOG:
          GNOME_Workrave_AppletControl_set_menu_status(applet_control, "/commands/ShowLog", active, &ev);
          break;
        }
      CORBA_exception_free(&ev);
    }
  TRACE_EXIT();
}


//! Retrieves the state of a toggle menu item.
bool
GnomeAppletWindow::get_menu_active(int menu)
{
  CORBA_Environment ev;
  bool ret = false;

  if (applet_control != NULL)
    {
      CORBA_exception_init (&ev);
      switch (menu)
        {
        case MENUSYNC_MODE_NORMAL:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Normal",
                                                             &ev);
          break;
        case MENUSYNC_MODE_SUSPENDED:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Suspended",
                                                             &ev);
          break;
        case MENUSYNC_MODE_QUIET:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/Quiet",
                                                             &ev);
          break;
        case MENUSYNC_SHOW_LOG:
          ret = GNOME_Workrave_AppletControl_get_menu_status(applet_control, "/commands/ShowLog",
                                                             &ev);
          break;
        }
      CORBA_exception_free(&ev);
    }
  return ret;
}


//! Sets the orientation of the applet.
void
GnomeAppletWindow::set_applet_orientation(Orientation o)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_orientation", o);

  applet_orientation = o;

  if (view != NULL)
    {
      view->set_geometry(applet_orientation, applet_size);
    }

  TRACE_EXIT();
}


//! Sets the size of the applet.
void
GnomeAppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_size", size);

  if (plug != NULL)
    {
      plug->queue_resize();
    }

  applet_size = size;

  if (view != NULL)
    {
      view->set_geometry(applet_orientation, applet_size);
    }

  TRACE_EXIT();
}


//! Sets the size of the applet.
void
GnomeAppletWindow::set_applet_background(int type, GdkColor &color, long xid)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_pixmap", xid);

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

//! Destroy notification.
gboolean
GnomeAppletWindow::destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  (void) widget;
  if (user_data != NULL)
    {
      GnomeAppletWindow *applet = (GnomeAppletWindow *) user_data;
      applet->delete_event(NULL);
    }
  return true;
}


//! User pressed some mouse button in the main window.
bool
GnomeAppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

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

  return ret;
}


//! User clicked left mouse button.
void
GnomeAppletWindow::button_clicked(int button)
{
  (void) button;
  if (timer_box_control != NULL)
    {
      timer_box_control->force_cycle();
    }
}


//! Notification of the system tray that the applet has been embedded.
void
GnomeAppletWindow::on_embedded()
{
  TRACE_ENTER("GnomeAppletWindow::on_embedded");
  control->set_applet_state(AppletControl::APPLET_GNOME,
                            AppletWindow::APPLET_STATE_VISIBLE);

  Menus *menus = Menus::get_instance();
  menus->resync();
  
  TRACE_EXIT();
}
