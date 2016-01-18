// GnomeAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "commonui/nls.h"
#include "debug.hh"

#include <gtkmm.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#if GTK_CHECK_VERSION(3, 0, 0)
#include <cairo.h>
#include <cairo-xlib.h>
#endif

#include <X11/X.h>
#include <X11/Xlib.h>

#include "GnomeAppletWindow.hh"

#include "TimerBoxGtkView.hh"
#include "commonui/TimerBoxControl.hh"
#include "Menus.hh"

#include "commonui/Backend.hh"

#include "dbus/IDBus.hh"

#include "Plug.hh"

#ifndef GDK_WINDOW_XWINDOW
#define GDK_WINDOW_XWINDOW(w) GDK_WINDOW_XID(w)
#endif


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
GnomeAppletWindow::GnomeAppletWindow() :
  view(NULL),
  plug(NULL),
  container(NULL),
  applet_orientation(ORIENTATION_UP),
  applet_size(0),
  applet_active(false),
  proxy(NULL)
{
}


//! Destructor.
GnomeAppletWindow::~GnomeAppletWindow()
{
  delete plug;
  delete container;
  delete timer_box_control;
  delete timer_box_view;

  cleanup_dbus();
}


void
GnomeAppletWindow::init_applet()
{
  try
    {
      workrave::dbus::IDBus::Ptr dbus = Backend::get_dbus();
      if (dbus->is_available())
        {
          dbus->connect("/org/workrave/Workrave/UI", "org.workrave.GnomeAppletSupportInterface", this);
        }

      init_dbus();
    }
  catch (workrave::dbus::DBusException)
    {
      cleanup_dbus();
    }
}

//! Initializes the native gnome applet.
AppletWindow::AppletState
GnomeAppletWindow::activate_applet()
{
  TRACE_ENTER("GnomeAppletWindow::activate_applet");
  bool ok = true;

  if (proxy == NULL)
    {
      TRACE_MSG("No proxy");
      ok = false;
    }

  if (ok && !applet_active)
    {
      long id = 0;

      try
        {
          TRACE_MSG("obtaining applet info");

          id = get_socketid();
          applet_size = get_size();
          applet_orientation = get_orientation();
        }
      catch (workrave::dbus::DBusException)
        {
          TRACE_MSG("exception");
          ok = false;
        }

      if (ok)
        {
          // Initialize applet GUI.

          Gtk::Alignment *frame = new Gtk::Alignment(0.0, 0.0, 0.0, 0.0);
          frame->set_border_width(0);
          container = frame;

          plug = new Plug(id);
          plug->add(*frame);

          plug->signal_size_allocate().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_plug_size_allocate));

          plug->set_events(plug->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

          plug->signal_embedded().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_embedded));
          plug->signal_delete_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::delete_event));

          // Gtkmm does not wrap this event....
          g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                           G_CALLBACK(GnomeAppletWindow::destroy_event), this);

          view = new TimerBoxGtkView(Menus::MENU_NONE, true);
          timer_box_view = view;
          timer_box_control = new TimerBoxControl("applet", timer_box_view);

          view->set_geometry(applet_orientation, applet_size);
          view->show_all();

          plug->signal_button_press_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_button_press_event));
          plug->signal_button_release_event().connect(sigc::mem_fun(*this, &GnomeAppletWindow::on_button_press_event));

          container->add(*view);
          container->show_all();
          plug->show_all();
          TRACE_MSG("showing plug");
        }
    }

  if (ok)
    {
      applet_active = true;
      TRACE_MSG("all ok");
    }

  TRACE_EXIT();
  return ok ?
    AppletWindow::APPLET_STATE_ACTIVE :
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
  state_changed_signal.emit(AppletWindow::APPLET_STATE_DISABLED);
  TRACE_EXIT();
  return true;
}


//! Fires up the applet (as requested by the native gnome applet).
void
GnomeAppletWindow::fire_gnome_applet()
{
  TRACE_ENTER("GnomeAppletWindow::fire_gnome_applet");
  request_activate_signal.emit();
  TRACE_EXIT();
}


//! Sets the state of a toggle menu item.
void
GnomeAppletWindow::set_menu_status(int menu, bool active)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_menu_active", menu << " " << active);

  try
    {
      switch (menu)
        {
        case MENUSYNC_MODE_NORMAL:
          set_menu_status("/commands/Normal", active);
          break;
        case MENUSYNC_MODE_SUSPENDED:
          set_menu_status("/commands/Suspended", active);
          break;
        case MENUSYNC_MODE_QUIET:
          set_menu_status("/commands/Quiet", active);
          break;
        case MENUSYNC_MODE_READING:
          set_menu_status("/commands/ReadingMode", active);
          break;
        }
    }
  catch(workrave::dbus::DBusException)
    {
    }

  TRACE_EXIT();
}


//! Sets the orientation of the applet.
void
GnomeAppletWindow::set_applet_orientation(Orientation o)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_orientation", o);

  if (applet_orientation != o)
    {
      applet_orientation = o;

      if (view != NULL)
        {
          view->set_geometry(applet_orientation, applet_size);
        }
    }
  TRACE_EXIT();
}


//! Sets the size of the applet.
void
GnomeAppletWindow::set_applet_size(int size)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_size", size);

  if (applet_size != size)
    {
      if (plug != NULL)
        {
          plug->queue_resize();
        }

      applet_size = size;

      if (view != NULL)
        {
          view->set_geometry(applet_orientation, applet_size);
        }
    }
  TRACE_EXIT();
}


#if GTK_CHECK_VERSION(3, 0, 0)

//! Sets the size of the applet.
void
GnomeAppletWindow::set_applet_background(int type, GdkColor &color, long xid)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_background", type << " " << xid
                  << " " << color.pixel
                  << " " << color.red
                  << " " << color.green
                  << " " << color.blue
                  );

  static GtkStyleProperties *properties = NULL;
  if (properties == NULL)
    {
      properties = gtk_style_properties_new();
    }

  if (plug == NULL)
    {
      return;
    }

  GtkWidget *widget = GTK_WIDGET(plug->gobj());
  gtk_widget_reset_style(widget);

  switch (type)
    {
    case 0: //PANEL_NO_BACKGROUND:
      gtk_style_context_remove_provider(gtk_widget_get_style_context(widget),
                                        GTK_STYLE_PROVIDER(properties));
      break;

    case 1: //PANEL_COLOR_BACKGROUND
        gtk_style_properties_set(properties, GTK_STATE_FLAG_NORMAL,
                                 "background-color", &color,
                                 "background-image", NULL,
                                 NULL);
      gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
                                     GTK_STYLE_PROVIDER(properties),
                                     GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      break;

    case 2: // PANEL_PIXMAP_BACKGROUND
      {

        //int width, height;

        gdk_error_trap_push();

        Display *dpy = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
        cairo_surface_t *surface = cairo_xlib_surface_create(dpy, xid, DefaultVisual(dpy, 0), 0, 0);
        cairo_pattern_t *pattern = cairo_pattern_create_for_surface(surface);

        if (pattern != NULL)
          {
            gtk_style_properties_set (properties, GTK_STATE_FLAG_NORMAL,
                                      "background-image", pattern,
                                      NULL);
            cairo_pattern_destroy(pattern);
            gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
                                           GTK_STYLE_PROVIDER(properties),
                                           GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
          }
        else
          {
            gtk_style_context_remove_provider(gtk_widget_get_style_context(widget),
                                              GTK_STYLE_PROVIDER(properties));
          }

        cairo_surface_destroy(surface);
        gdk_flush();
        (void) gdk_error_trap_pop();
      }
      break;

    default:
      g_assert_not_reached ();
      break;
    }
  TRACE_EXIT();
}

#else

void
GnomeAppletWindow::set_applet_background(int type, GdkColor &color, long xid)
{
  TRACE_ENTER_MSG("GnomeAppletWindow::set_applet_background", type << " " << xid
                  << " " << color.pixel
                  << " " << color.red
                  << " " << color.green
                  << " " << color.blue
                  );
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
          GdkWindow *rootwin = gdk_get_default_root_window();
          GdkColormap *cmap = gdk_drawable_get_colormap(GDK_DRAWABLE(rootwin));

          gdk_pixbuf_get_from_drawable (pbuf, orig_pixmap, cmap, 0, 0,
                                        0, 0, width , height);

          /* put background onto the widget */
          gdk_pixbuf_render_pixmap_and_mask(pbuf, &pixmap, NULL, 127);

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

  GtkRcStyle *rc_style = NULL;
  GtkStyle *style = NULL;

  gtk_widget_set_style(widget, NULL);
  rc_style = gtk_rc_style_new();
  gtk_widget_modify_style (widget, rc_style);
  g_object_unref(rc_style);

  switch (type)
    {
    case 0: //PANEL_NO_BACKGROUND:
      break;
    case 1: //PANEL_COLOR_BACKGROUND:
      gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color);
      break;
    case 2: //PANEL_PIXMAP_BACKGROUND:
      style = gtk_style_copy(widget->style);
      if (style->bg_pixmap[GTK_STATE_NORMAL])
        g_object_unref(style->bg_pixmap[GTK_STATE_NORMAL]);
      style->bg_pixmap[GTK_STATE_NORMAL] = (GdkPixmap *)g_object_ref(pixmap);
      gtk_widget_set_style(widget, style);
      g_object_unref(style);
      break;
    }

  if (pixmap != NULL)
    {
      g_object_unref(G_OBJECT(pixmap));
    }

  TRACE_EXIT();
}
#endif

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


void
GnomeAppletWindow::on_plug_size_allocate(Gtk::Allocation &allocation)
{
  TRACE_ENTER("GnomeAppletWindow::on_plug_size_allocate");
  TRACE_MSG("alloc " << allocation.get_width() << " " << allocation.get_height());

  if ((applet_orientation == ORIENTATION_LEFT || applet_orientation == ORIENTATION_RIGHT))
    {
      set_applet_size(allocation.get_width());
    }
  else
    {
      set_applet_size(allocation.get_height());
    }
  TRACE_EXIT();
}

//! User pressed some mouse button in the main window.
bool
GnomeAppletWindow::on_button_press_event(GdkEventButton *event)
{
  TRACE_ENTER("GnomeAppletWindow::on_button_press_event");
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

#ifdef HAVE_GTK3
      GdkDeviceManager *device_manager = gdk_display_get_device_manager(gtk_widget_get_display(widget));
      GList *devices = gdk_device_manager_list_devices(device_manager, GDK_DEVICE_TYPE_MASTER);

      for (GList *d = devices; d; d = d->next)
        {
          GdkDevice *device = (GdkDevice *)d->data;

          if (gdk_device_get_source(device) == GDK_SOURCE_MOUSE)
            {
              gdk_device_ungrab(device, GDK_CURRENT_TIME);
            }
        }

      g_list_free(devices);
#else
      gdk_display_pointer_ungrab(gtk_widget_get_display(widget), GDK_CURRENT_TIME);
#endif
      ok = true;
    }
  else if (event->type == GDK_BUTTON_RELEASE)
    {
      TRACE_MSG("release");
      xevent.xbutton.type = ButtonRelease;
      ok = true;
    }

  if (ok)
    {
      GdkScreen *screen = gtk_widget_get_screen(widget);

      xevent.xbutton.display     = GDK_WINDOW_XDISPLAY(gtk_widget_get_window(widget));
      xevent.xbutton.window      = GDK_WINDOW_XWINDOW(gtk_plug_get_socket_window(GTK_PLUG(widget)));
      xevent.xbutton.root        = GDK_WINDOW_XWINDOW(gdk_screen_get_root_window(screen));

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

      TRACE_MSG("send");

      XSendEvent(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(widget)),
                 GDK_WINDOW_XWINDOW(gtk_plug_get_socket_window(GTK_PLUG(widget))),
                 False, NoEventMask, &xevent);

      gdk_flush();
      gint err = gdk_error_trap_pop();
      (void) err;
    }

  TRACE_EXIT();
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
  state_changed_signal.emit(AppletWindow::APPLET_STATE_ACTIVE);
  TRACE_EXIT();
}

void
GnomeAppletWindow::cleanup_dbus()
{
  if (proxy != NULL)
    {
      g_object_unref(proxy);
      proxy = NULL;
    }
}

void
GnomeAppletWindow::init_dbus()
{
  TRACE_ENTER("GnomeAppletWindow::init_dbus");
  GError *error = NULL;

  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        "org.workrave.Workrave.GnomeApplet",
                                        "/org/workrave/Workrave/GnomeApplet",
                                        "org.workrave.GnomeAppletInterface",
                                        NULL,
                                        &error);

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }

  TRACE_EXIT();
}

guint32
GnomeAppletWindow::get_socketid()
{
  guint32 ret = 0;
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "GetSocketId",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          g_error_free(error);
          throw workrave::dbus::DBusException("Cannot get socket id");
        }

      g_variant_get(result, "(u)", &ret);
      g_variant_unref(result);
    }
  return ret;
}

guint32
GnomeAppletWindow::get_size()
{
  guint32 ret = 0;
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "GetSize",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          g_error_free(error);
          throw workrave::dbus::DBusException("Cannot get size");
        }

      g_variant_get(result, "(u)", &ret);
      g_variant_unref(result);
    }
  return ret;
}

Orientation
GnomeAppletWindow::get_orientation()
{
  guint32 ret = 0;
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "GetOrientation",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          g_error_free(error);
          throw workrave::dbus::DBusException("Cannot get socket id");
        }

      g_variant_get(result, "(u)", &ret);
      g_variant_unref(result);
    }
  return (Orientation)ret;
}

void
GnomeAppletWindow::set_menu_status(const std::string &menu, bool status)
{
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "SetMenuStatus",
                                                g_variant_new("(sb)", menu.c_str(), status),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          g_error_free(error);
          throw workrave::dbus::DBusException("Cannot set menu status");
        }

      g_variant_unref(result);
    }
}

void
GnomeAppletWindow::set_menu_active(const std::string &menu, bool active)
{
  if (proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "SetMenuActive",
                                                g_variant_new("(sb)", menu.c_str(), active),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      if (error != NULL)
        {
          g_error_free(error);
          throw workrave::dbus::DBusException("Cannot set menu active");
        }
      g_variant_unref(result);
    }
}
