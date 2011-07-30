/* gtktrayicon.c
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * This is an implementation of the freedesktop.org "system tray" spec,
 * http://www.freedesktop.org/wiki/Standards/systemtray-spec
 */

/*
 * Ripped from Gtk+ because it is not a public API.
 * Workrave abuses the trayicon by putting more than a simple icon
 * in the tray. Gtk+ only has a statusicon as public API.
 *
 * Renamed to avoid naming conflicts.
 *
 * -- Rob Caelers
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"

#include <string.h>

#include "gtktrayicon.h"

#include <gdk/gdkx.h>
#include <X11/Xatom.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

enum {
  PROP_0,
  PROP_ORIENTATION
};

struct _WRGtkTrayIconPrivate
{
  guint stamp;

  Atom selection_atom;
  Atom manager_atom;
  Atom system_tray_opcode_atom;
  Atom orientation_atom;
  Atom visual_atom;
  Window manager_window;
  GdkVisual *manager_visual;
  gboolean manager_visual_rgba;

  GtkOrientation orientation;
};

static void wrgtk_tray_icon_constructed   (GObject     *object);
static void wrgtk_tray_icon_dispose       (GObject     *object);

static void wrgtk_tray_icon_get_property  (GObject     *object,
				 	 guint        prop_id,
					 GValue      *value,
					 GParamSpec  *pspec);

static void     wrgtk_tray_icon_realize   (GtkWidget   *widget);
static void     wrgtk_tray_icon_style_set (GtkWidget   *widget,
					 GtkStyle    *previous_style);
static gboolean wrgtk_tray_icon_delete    (GtkWidget   *widget,
					 GdkEventAny *event);
static gboolean wrgtk_tray_icon_expose    (GtkWidget      *widget,
					 GdkEventExpose *event);

static void wrgtk_tray_icon_clear_manager_window     (WRGtkTrayIcon *icon);
static void wrgtk_tray_icon_update_manager_window    (WRGtkTrayIcon *icon);
static void wrgtk_tray_icon_manager_window_destroyed (WRGtkTrayIcon *icon);

static GdkFilterReturn wrgtk_tray_icon_manager_filter (GdkXEvent *xevent,
						     GdkEvent  *event,
						     gpointer   user_data);


G_DEFINE_TYPE (WRGtkTrayIcon, wrgtk_tray_icon, GTK_TYPE_PLUG)

static void
wrgtk_tray_icon_class_init (WRGtkTrayIconClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;
  GtkWidgetClass *widget_class = (GtkWidgetClass *)class;

  gobject_class->get_property = wrgtk_tray_icon_get_property;
  gobject_class->constructed = wrgtk_tray_icon_constructed;
  gobject_class->dispose = wrgtk_tray_icon_dispose;

  widget_class->realize = wrgtk_tray_icon_realize;
  widget_class->style_set = wrgtk_tray_icon_style_set;
  widget_class->delete_event = wrgtk_tray_icon_delete;
  widget_class->expose_event = wrgtk_tray_icon_expose;

  g_object_class_install_property (gobject_class,
				   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
						      _("Orientation"),
						      _("The orientation of the tray"),
						      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_HORIZONTAL,
						      G_PARAM_READABLE));

  g_type_class_add_private (class, sizeof (WRGtkTrayIconPrivate));
}

static void
wrgtk_tray_icon_init (WRGtkTrayIcon *icon)
{
  icon->priv = G_TYPE_INSTANCE_GET_PRIVATE (icon, GTK_TYPE_TRAY_ICON,
					    WRGtkTrayIconPrivate);

  icon->priv->stamp = 1;
  icon->priv->orientation = GTK_ORIENTATION_HORIZONTAL;

  gtk_widget_set_app_paintable (GTK_WIDGET (icon), TRUE);
  gtk_widget_add_events (GTK_WIDGET (icon), GDK_PROPERTY_CHANGE_MASK);
}

static void
wrgtk_tray_icon_constructed (GObject *object)
{
  /* Do setup that depends on the screen; screen has been set at this point */

  WRGtkTrayIcon *icon = GTK_TRAY_ICON (object);
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (object));
  GdkWindow *root_window = gdk_screen_get_root_window (screen);
  GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (object));
  Display *xdisplay = gdk_x11_display_get_xdisplay (display);
  char buffer[256];

  g_snprintf (buffer, sizeof (buffer),
	      "_NET_SYSTEM_TRAY_S%d",
	      gdk_screen_get_number (screen));

  icon->priv->selection_atom = XInternAtom (xdisplay, buffer, False);

  icon->priv->manager_atom = XInternAtom (xdisplay, "MANAGER", False);

  icon->priv->system_tray_opcode_atom = XInternAtom (xdisplay,
						     "_NET_SYSTEM_TRAY_OPCODE",
						     False);

  icon->priv->orientation_atom = XInternAtom (xdisplay,
					      "_NET_SYSTEM_TRAY_ORIENTATION",
					      False);

  icon->priv->visual_atom = XInternAtom (xdisplay,
					 "_NET_SYSTEM_TRAY_VISUAL",
					 False);

  /* Add a root window filter so that we get changes on MANAGER */
  gdk_window_add_filter (root_window,
			 wrgtk_tray_icon_manager_filter, icon);

  wrgtk_tray_icon_update_manager_window (icon);
}

static void
wrgtk_tray_icon_clear_manager_window (WRGtkTrayIcon *icon)
{
  GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (icon));

  if (icon->priv->manager_window != None)
    {
      GdkWindow *gdkwin;

      gdkwin = gdk_window_lookup_for_display (display,
                                              icon->priv->manager_window);

      gdk_window_remove_filter (gdkwin, wrgtk_tray_icon_manager_filter, icon);

      icon->priv->manager_window = None;
      icon->priv->manager_visual = NULL;
    }
}

static void
wrgtk_tray_icon_dispose (GObject *object)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (object);
  GtkWidget *widget = GTK_WIDGET (object);
  GdkWindow *root_window = gdk_screen_get_root_window (gtk_widget_get_screen (widget));

  wrgtk_tray_icon_clear_manager_window (icon);

  gdk_window_remove_filter (root_window, wrgtk_tray_icon_manager_filter, icon);
}

static void
wrgtk_tray_icon_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (object);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, icon->priv->orientation);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
wrgtk_tray_icon_expose (GtkWidget      *widget,
		      GdkEventExpose *event)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);
  GtkWidget *focus_child;
  gint border_width, x, y, width, height;
  gboolean retval = FALSE;

  if (icon->priv->manager_visual_rgba)
    {
      /* Clear to transparent */
      cairo_t *cr = gdk_cairo_create (widget->window);
      cairo_set_source_rgba (cr, 0, 0, 0, 0);
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      gdk_cairo_region (cr, event->region);
      cairo_fill (cr);
      cairo_destroy (cr);
    }
  else
    {
      /* Clear to parent-relative pixmap */
      gdk_window_clear_area (widget->window, event->area.x, event->area.y,
			     event->area.width, event->area.height);
    }

  if (GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->expose_event)
    retval = GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->expose_event (widget, event);

  focus_child = GTK_CONTAINER (widget)->focus_child;
  if (focus_child && GTK_WIDGET_HAS_FOCUS (focus_child))
    {
      border_width = GTK_CONTAINER (widget)->border_width;

      x = widget->allocation.x + border_width;
      y = widget->allocation.y + border_width;

      width  = widget->allocation.width  - 2 * border_width;
      height = widget->allocation.height - 2 * border_width;

      gtk_paint_focus (widget->style, widget->window,
                       GTK_WIDGET_STATE (widget),
                       &event->area, widget, "tray_icon",
                       x, y, width, height);
    }

  return retval;
}

static void
wrgtk_tray_icon_get_orientation_property (WRGtkTrayIcon *icon)
{
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (icon));
  GdkDisplay *display = gdk_screen_get_display (screen);
  Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);

  Atom type;
  int format;
  union {
	gulong *prop;
	guchar *prop_ch;
  } prop = { NULL };
  gulong nitems;
  gulong bytes_after;
  int error, result;

  g_assert (icon->priv->manager_window != None);

  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (xdisplay,
			       icon->priv->manager_window,
			       icon->priv->orientation_atom,
			       0, G_MAXLONG, FALSE,
			       XA_CARDINAL,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (error || result != Success)
    return;

  if (type == XA_CARDINAL && nitems == 1 && format == 32)
    {
      GtkOrientation orientation;

      orientation = (prop.prop [0] == SYSTEM_TRAY_ORIENTATION_HORZ) ?
					GTK_ORIENTATION_HORIZONTAL :
					GTK_ORIENTATION_VERTICAL;

      if (icon->priv->orientation != orientation)
	{
	  icon->priv->orientation = orientation;

	  g_object_notify (G_OBJECT (icon), "orientation");
	}
    }

  if (type != None)
    XFree (prop.prop);
}

void
wrgtk_tray_icon_get_visual_property (WRGtkTrayIcon *icon)
{
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (icon));
  GdkDisplay *display = gdk_screen_get_display (screen);
  Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);

  Atom type;
  int format;
  union {
	gulong *prop;
	guchar *prop_ch;
  } prop = { NULL };
  gulong nitems;
  gulong bytes_after;
  int error, result;
  GdkVisual *visual;

  g_assert (icon->priv->manager_window != None);

  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (xdisplay,
			       icon->priv->manager_window,
			       icon->priv->visual_atom,
			       0, G_MAXLONG, FALSE,
			       XA_VISUALID,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  visual = NULL;

  if (!error && result == Success &&
      type == XA_VISUALID && nitems == 1 && format == 32)
    {
      VisualID visual_id = prop.prop[0];
      visual = gdk_x11_screen_lookup_visual (screen, visual_id);
    }

  icon->priv->manager_visual = visual;
  icon->priv->manager_visual_rgba = visual != NULL &&
    (visual->red_prec + visual->blue_prec + visual->green_prec < visual->depth);

  /* For the background-relative hack we use when we aren't using a real RGBA
   * visual, we can't be double-buffered */
  gtk_widget_set_double_buffered (GTK_WIDGET (icon), icon->priv->manager_visual_rgba);

  if (type != None)
    XFree (prop.prop);
}

static GdkFilterReturn
wrgtk_tray_icon_manager_filter (GdkXEvent *xevent,
			      GdkEvent  *event,
			      gpointer   user_data)
{
  WRGtkTrayIcon *icon = user_data;
  XEvent *xev = (XEvent *)xevent;

  if (xev->xany.type == ClientMessage &&
      xev->xclient.message_type == icon->priv->manager_atom &&
      xev->xclient.data.l[1] == icon->priv->selection_atom)
    {
      GTK_NOTE (PLUGSOCKET,
		g_print ("GtkStatusIcon %p: tray manager appeared\n", icon));

      wrgtk_tray_icon_update_manager_window (icon);
    }
  else if (xev->xany.window == icon->priv->manager_window)
    {
      if (xev->xany.type == PropertyNotify &&
	  xev->xproperty.atom == icon->priv->orientation_atom)
	{
          GTK_NOTE (PLUGSOCKET,
		    g_print ("GtkStatusIcon %p: got PropertyNotify on manager window for orientation atom\n", icon));

	  wrgtk_tray_icon_get_orientation_property (icon);
	}
      else if (xev->xany.type == DestroyNotify)
	{
          GTK_NOTE (PLUGSOCKET,
		    g_print ("GtkStatusIcon %p: got DestroyNotify for manager window\n", icon));

	  wrgtk_tray_icon_manager_window_destroyed (icon);
	}
      else
        GTK_NOTE (PLUGSOCKET,
		  g_print ("GtkStatusIcon %p: got other message on manager window\n", icon));
    }

  return GDK_FILTER_CONTINUE;
}

static void
wrgtk_tray_icon_send_manager_message (WRGtkTrayIcon *icon,
				    long         message,
				    Window       window,
				    long         data1,
				    long         data2,
				    long         data3)
{
  XClientMessageEvent ev;
  Display *display;

  memset (&ev, 0, sizeof (ev));
  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->priv->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (GTK_WIDGET (icon)->window);
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

  display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

  gdk_error_trap_push ();
  XSendEvent (display,
	      icon->priv->manager_window, False, NoEventMask, (XEvent *)&ev);
  XSync (display, False);
  gdk_error_trap_pop ();
}

static void
wrgtk_tray_icon_send_dock_request (WRGtkTrayIcon *icon)
{
  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: sending dock request to manager window %lx\n",
	    	     icon, (gulong) icon->priv->manager_window));

  wrgtk_tray_icon_send_manager_message (icon,
				      SYSTEM_TRAY_REQUEST_DOCK,
				      icon->priv->manager_window,
				      gtk_plug_get_id (GTK_PLUG (icon)),
				      0, 0);
}

static void
wrgtk_tray_icon_update_manager_window (WRGtkTrayIcon *icon)
{
  GtkWidget *widget = GTK_WIDGET (icon);
  GdkScreen *screen = gtk_widget_get_screen (widget);
  GdkDisplay *display = gdk_screen_get_display (screen);
  Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: updating tray icon manager window, current manager window: %lx\n",
		     icon, (gulong) icon->priv->manager_window));

  if (icon->priv->manager_window != None)
    return;

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: trying to find manager window\n", icon));

  XGrabServer (xdisplay);

  icon->priv->manager_window = XGetSelectionOwner (xdisplay,
						   icon->priv->selection_atom);

  if (icon->priv->manager_window != None)
    XSelectInput (xdisplay,
		  icon->priv->manager_window, StructureNotifyMask|PropertyChangeMask);

  XUngrabServer (xdisplay);
  XFlush (xdisplay);

  if (icon->priv->manager_window != None)
    {
      GdkWindow *gdkwin;

      GTK_NOTE (PLUGSOCKET,
		g_print ("GtkStatusIcon %p: is being managed by window %lx\n",
				icon, (gulong) icon->priv->manager_window));

      gdkwin = gdk_window_lookup_for_display (display,
					      icon->priv->manager_window);

      gdk_window_add_filter (gdkwin, wrgtk_tray_icon_manager_filter, icon);

      wrgtk_tray_icon_get_orientation_property (icon);
      wrgtk_tray_icon_get_visual_property (icon);

      if (GTK_WIDGET_REALIZED (icon))
	{
	  if ((icon->priv->manager_visual == NULL &&
	       gtk_widget_get_visual (widget) == gdk_screen_get_system_visual (screen)) ||
	      (icon->priv->manager_visual == gtk_widget_get_visual (widget)))
	    {
	      /* Already have the right visual, can just dock
	       */
	      wrgtk_tray_icon_send_dock_request (icon);
	    }
	  else
	    {
	      /* Need to re-realize the widget to get the right visual
	       */
	      gtk_widget_hide (widget);
	      gtk_widget_unrealize (widget);
	      gtk_widget_show (widget);
	    }
	}
    }
  else
    GTK_NOTE (PLUGSOCKET,
	      g_print ("GtkStatusIcon %p: no tray manager found\n", icon));
}

static void
wrgtk_tray_icon_manager_window_destroyed (WRGtkTrayIcon *icon)
{
  g_return_if_fail (icon->priv->manager_window != None);

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: tray manager window destroyed\n", icon));

  wrgtk_tray_icon_clear_manager_window (icon);
}

static gboolean
wrgtk_tray_icon_delete (GtkWidget   *widget,
		      GdkEventAny *event)
{
  // WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: delete notify, tray manager window %lx\n",
		     icon, (gulong) icon->priv->manager_window));

  /* A bug in X server versions up to x.org 1.5.0 means that:
   * XFixesChangeSaveSet(...., SaveSetRoot, SaveSetUnmap) doesn't work properly
   * and we'll left mapped in a separate toplevel window if the tray is destroyed.
   * For simplicity just get rid of our X window and start over.
   */
  gtk_widget_hide (widget);
  gtk_widget_unrealize (widget);
  gtk_widget_show (widget);

  /* Handled it, don't destroy the tray icon */
  return TRUE;
}

static void
wrgtk_tray_icon_set_colormap (WRGtkTrayIcon *icon)
{
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (icon));
  GdkColormap *colormap;
  GdkVisual *visual = icon->priv->manager_visual;
  gboolean new_colormap = FALSE;

  /* To avoid uncertainty about colormaps, _NET_SYSTEM_TRAY_VISUAL is supposed
   * to be either the screen default visual or a TrueColor visual; ignore it
   * if it is something else
   */
  if (visual && visual->type != GDK_VISUAL_TRUE_COLOR)
    visual = NULL;

  if (visual == NULL || visual == gdk_screen_get_system_visual (screen))
    colormap = gdk_screen_get_system_colormap (screen);
  else if (visual == gdk_screen_get_rgb_visual (screen))
    colormap = gdk_screen_get_rgb_colormap (screen);
  else if (visual == gdk_screen_get_rgba_visual (screen))
    colormap = gdk_screen_get_rgba_colormap (screen);
  else
    {
      colormap = gdk_colormap_new (visual, FALSE);
      new_colormap = TRUE;
    }

  gtk_widget_set_colormap (GTK_WIDGET (icon), colormap);

  if (new_colormap)
    g_object_unref (colormap);
}

static void
wrgtk_tray_icon_realize (GtkWidget *widget)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);

  /* Set our colormap before realizing */
  wrgtk_tray_icon_set_colormap (icon);

  GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->realize (widget);
  if (icon->priv->manager_visual_rgba)
    {
      /* Set a transparent background */
      GdkColor transparent = { 0, 0, 0, 0 }; /* Only pixel=0 matters */
      gdk_window_set_background (widget->window, &transparent);
    }
  else
    {
      /* Set a parent-relative background pixmap */
      gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
    }

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: realized, window: %lx, socket window: %lx\n",
		     widget,
		     (gulong) GDK_WINDOW_XWINDOW (widget->window),
		     GTK_PLUG (icon)->socket_window ?
			     (gulong) GDK_WINDOW_XWINDOW (GTK_PLUG (icon)->socket_window) : 0UL));

  if (icon->priv->manager_window != None)
    wrgtk_tray_icon_send_dock_request (icon);
}

static void
wrgtk_tray_icon_style_set (GtkWidget   *widget,
			 GtkStyle    *previous_style)
{
  /* The default handler resets the background according to the style. We either
   * use a transparent background or a parent-relative background and ignore the
   * style background. So, just don't chain up.
   */
}

guint
_wrgtk_tray_icon_send_message (WRGtkTrayIcon *icon,
			     gint         timeout,
			     const gchar *message,
			     gint         len)
{
  guint stamp;

  g_return_val_if_fail (GTK_IS_TRAY_ICON (icon), 0);
  g_return_val_if_fail (timeout >= 0, 0);
  g_return_val_if_fail (message != NULL, 0);

  if (icon->priv->manager_window == None)
    return 0;

  if (len < 0)
    len = strlen (message);

  stamp = icon->priv->stamp++;

  /* Get ready to send the message */
  wrgtk_tray_icon_send_manager_message (icon, SYSTEM_TRAY_BEGIN_MESSAGE,
				      (Window)gtk_plug_get_id (GTK_PLUG (icon)),
				      timeout, len, stamp);

  /* Now to send the actual message */
  gdk_error_trap_push ();
  while (len > 0)
    {
      XClientMessageEvent ev;
      Display *xdisplay;

      xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));

      memset (&ev, 0, sizeof (ev));
      ev.type = ClientMessage;
      ev.window = (Window)gtk_plug_get_id (GTK_PLUG (icon));
      ev.format = 8;
      ev.message_type = XInternAtom (xdisplay,
				     "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);
      if (len > 20)
	{
	  memcpy (&ev.data, message, 20);
	  len -= 20;
	  message += 20;
	}
      else
	{
	  memcpy (&ev.data, message, len);
	  len = 0;
	}

      XSendEvent (xdisplay,
		  icon->priv->manager_window, False,
		  StructureNotifyMask, (XEvent *)&ev);
      XSync (xdisplay, False);
    }

  gdk_error_trap_pop ();

  return stamp;
}

void
_wrgtk_tray_icon_cancel_message (WRGtkTrayIcon *icon,
			       guint        id)
{
  g_return_if_fail (GTK_IS_TRAY_ICON (icon));
  g_return_if_fail (id > 0);

  wrgtk_tray_icon_send_manager_message (icon, SYSTEM_TRAY_CANCEL_MESSAGE,
				      (Window)gtk_plug_get_id (GTK_PLUG (icon)),
				      id, 0, 0);
}

WRGtkTrayIcon *
wrgtk_tray_icon_new_for_screen (GdkScreen  *screen,
			       const gchar *name)
{
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  return g_object_new (GTK_TYPE_TRAY_ICON,
		       "screen", screen,
		       "title", name,
		       NULL);
}

WRGtkTrayIcon*
wrgtk_tray_icon_new (const gchar *name)
{
  return g_object_new (GTK_TYPE_TRAY_ICON,
		       "title", name,
		       NULL);
}

GtkOrientation
wrgtk_tray_icon_get_orientation (WRGtkTrayIcon *icon)
{
  g_return_val_if_fail (GTK_IS_TRAY_ICON (icon), GTK_ORIENTATION_HORIZONTAL);

  return icon->priv->orientation;
}
