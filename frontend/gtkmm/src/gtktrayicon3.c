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

#include "config.h"

#include <math.h>
#include <string.h>

#include "nls.h"

#include <X11/Xatom.h>
#include <cairo-xlib.h>

#include "gtk/gtk.h"
#include "gtk/gtkx.h"
#include "gdk/gdk.h"

//#include "gtkintl.h"
//#include "gtkprivate.h"
#include "gtktrayicon.h"
//#include "gtktestutils.h"
//#include "gtkdebug.h"
//#include "gtktypebuiltins.h"

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define SYSTEM_TRAY_ORIENTATION_HORZ 0
#define SYSTEM_TRAY_ORIENTATION_VERT 1

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_FG_COLOR,
  PROP_ERROR_COLOR,
  PROP_WARNING_COLOR,
  PROP_SUCCESS_COLOR,
  PROP_PADDING
};

struct _WRGtkTrayIconPrivate
{
  guint stamp;
  
  Atom selection_atom;
  Atom manager_atom;
  Atom system_tray_opcode_atom;
  Atom orientation_atom;
  Atom visual_atom;
  Atom colors_atom;
  Atom padding_atom;
  Window manager_window;
  GdkVisual *manager_visual;
  gboolean manager_visual_rgba;

  GtkOrientation orientation;
  GdkColor fg_color;
  GdkColor error_color;
  GdkColor warning_color;
  GdkColor success_color;
  gint padding;
};

static void wrgtk_tray_icon_constructed   (GObject     *object);
static void wrgtk_tray_icon_dispose       (GObject     *object);

static void wrgtk_tray_icon_get_property  (GObject     *object,
				 	 guint        prop_id,
					 GValue      *value,
					 GParamSpec  *pspec);

static void     wrgtk_tray_icon_realize   (GtkWidget   *widget);
static void     wrgtk_tray_icon_style_updated (GtkWidget   *widget);
static gboolean wrgtk_tray_icon_delete    (GtkWidget   *widget,
					 GdkEventAny *event);
static gboolean wrgtk_tray_icon_draw      (GtkWidget   *widget,
                                         cairo_t     *cr);

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
  widget_class->style_updated = wrgtk_tray_icon_style_updated;
  widget_class->delete_event = wrgtk_tray_icon_delete;
  widget_class->draw = wrgtk_tray_icon_draw;

  g_object_class_install_property (gobject_class,
				   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
						      _("Orientation"),
						      _("The orientation of the tray"),
						      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_HORIZONTAL,
						      G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_FG_COLOR,
                                   g_param_spec_boxed ("fg-color",
                                                       _("Foreground color"),
                                                       _("Foreground color for symbolic icons"),
                                                       GDK_TYPE_COLOR,
                                                       G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_ERROR_COLOR,
                                   g_param_spec_boxed ("error-color",
                                                       _("Error color"),
                                                       _("Error color for symbolic icons"),
                                                       GDK_TYPE_COLOR,
                                                       G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_WARNING_COLOR,
                                   g_param_spec_boxed ("warning-color",
                                                       _("Warning color"),
                                                       _("Warning color for symbolic icons"),
                                                       GDK_TYPE_COLOR,
                                                       G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
                                   PROP_SUCCESS_COLOR,
                                   g_param_spec_boxed ("success-color",
                                                       _("Success color"),
                                                       _("Success color for symbolic icons"),
                                                       GDK_TYPE_COLOR,
                                                       G_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
				   PROP_PADDING,
				   g_param_spec_int ("padding",
						     _("Padding"),
						     _("Padding that should be put around icons in the tray"),
						     0,
                                                     G_MAXINT,
                                                     0,
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
  icon->priv->fg_color.red        = 0x0000;
  icon->priv->fg_color.green      = 0x0000;
  icon->priv->fg_color.blue       = 0x0000;
  icon->priv->error_color.red     = 0xcc00;
  icon->priv->error_color.green   = 0x0000;
  icon->priv->error_color.blue    = 0x0000;
  icon->priv->warning_color.red   = 0xf500;
  icon->priv->warning_color.green = 0x7900;
  icon->priv->warning_color.blue  = 0x3e00;
  icon->priv->success_color.red   = 0x4e00;
  icon->priv->success_color.green = 0x9a00;
  icon->priv->success_color.blue  = 0x0600;
  icon->priv->padding = 0;

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

  icon->priv->colors_atom = XInternAtom (xdisplay,
                                         "_NET_SYSTEM_TRAY_COLORS",
                                         False);

  icon->priv->padding_atom = XInternAtom (xdisplay,
					 "_NET_SYSTEM_TRAY_PADDING",
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

      gdkwin = gdk_x11_window_lookup_for_display (display,
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
    case PROP_FG_COLOR:
      g_value_set_boxed (value, &icon->priv->fg_color);
      break;
    case PROP_ERROR_COLOR:
      g_value_set_boxed (value, &icon->priv->error_color);
      break;
    case PROP_WARNING_COLOR:
      g_value_set_boxed (value, &icon->priv->warning_color);
      break;
    case PROP_SUCCESS_COLOR:
      g_value_set_boxed (value, &icon->priv->success_color);
      break;
    case PROP_PADDING:
      g_value_set_int (value, icon->priv->padding);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
wrgtk_tray_icon_draw (GtkWidget *widget, 
		    cairo_t   *cr)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);
  GtkWidget *focus_child;
  GdkWindow *window;
  gint border_width;
  gboolean retval = FALSE;
  cairo_surface_t *target;

  window = gtk_widget_get_window (widget);
  target = cairo_get_group_target (cr);

  if (icon->priv->manager_visual_rgba ||
      cairo_surface_get_type (target) != CAIRO_SURFACE_TYPE_XLIB ||
      cairo_xlib_surface_get_drawable (target) != GDK_WINDOW_XID (window))
    {
      /* Clear to transparent */
      cairo_set_source_rgba (cr, 0, 0, 0, 0);
      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_paint (cr);
    }
  else
    {
      GdkRectangle clip;

      if (gdk_cairo_get_clip_rectangle (cr, &clip))
        {
          /* Clear to parent-relative pixmap
           * We need to use direct X access here because GDK doesn't know about
           * the parent realtive pixmap. */
          cairo_surface_flush (target);

          XClearArea (GDK_WINDOW_XDISPLAY (window),
                      GDK_WINDOW_XID (window),
                      clip.x, clip.y,
                      clip.width, clip.height,
                      False);
          cairo_surface_mark_dirty_rectangle (target, 
                                              clip.x, clip.y,
                                              clip.width, clip.height);
        }
    }

  if (GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->draw)
    retval = GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->draw (widget, cr);

  focus_child = gtk_container_get_focus_child (GTK_CONTAINER (widget));
  if (focus_child && gtk_widget_has_focus (focus_child))
    {
      GtkStyleContext *context;
      GtkStateFlags state;

      border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));
      context = gtk_widget_get_style_context (widget);
      state = gtk_widget_get_state_flags (widget);

      gtk_style_context_save (context);
      gtk_style_context_set_state (context, state);

      gtk_render_focus (context, cr, border_width, border_width,
                        gtk_widget_get_allocated_width (widget) - 2 * border_width,
                        gtk_widget_get_allocated_height (widget) - 2 * border_width);

      gtk_style_context_restore (context);
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

static void
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
  gint red_prec;
  gint green_prec;
  gint blue_prec;

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

  gdk_visual_get_red_pixel_details (visual, NULL, NULL, &red_prec);
  gdk_visual_get_green_pixel_details (visual, NULL, NULL, &green_prec);
  gdk_visual_get_blue_pixel_details (visual, NULL, NULL, &blue_prec);

  icon->priv->manager_visual = visual;
  icon->priv->manager_visual_rgba = visual != NULL &&
    (red_prec + blue_prec + green_prec < gdk_visual_get_depth (visual));

  /* For the background-relative hack we use when we aren't using a real RGBA
   * visual, we can't be double-buffered */
  gtk_widget_set_double_buffered (GTK_WIDGET (icon), icon->priv->manager_visual_rgba);

  if (type != None)
    XFree (prop.prop);
}

static void
wrgtk_tray_icon_get_colors_property (WRGtkTrayIcon *icon)
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
			       icon->priv->colors_atom,
			       0, G_MAXLONG, FALSE,
			       XA_CARDINAL,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (error || result != Success)
    return;

  if (type == XA_CARDINAL && nitems == 12 && format == 32)
    {
      GdkColor color;

      g_object_freeze_notify (G_OBJECT (icon));

      color.red = prop.prop[0];
      color.green = prop.prop[1];
      color.blue = prop.prop[2];

      if (!gdk_color_equal (&icon->priv->fg_color, &color))
        {
          icon->priv->fg_color = color;

          g_object_notify (G_OBJECT (icon), "fg-color");
        }

      color.red = prop.prop[3];
      color.green = prop.prop[4];
      color.blue = prop.prop[5];

      if (!gdk_color_equal (&icon->priv->error_color, &color))
        {
          icon->priv->error_color = color;

          g_object_notify (G_OBJECT (icon), "error-color");
        }

      color.red = prop.prop[6];
      color.green = prop.prop[7];
      color.blue = prop.prop[8];

      if (!gdk_color_equal (&icon->priv->warning_color, &color))
        {
          icon->priv->warning_color = color;

          g_object_notify (G_OBJECT (icon), "warning-color");
        }

      color.red = prop.prop[9];
      color.green = prop.prop[10];
      color.blue = prop.prop[11];

      if (!gdk_color_equal (&icon->priv->success_color, &color))
        {
          icon->priv->success_color = color;

          g_object_notify (G_OBJECT (icon), "success-color");
        }

      g_object_thaw_notify (G_OBJECT (icon));
    }

  if (type != None)
    XFree (prop.prop);
}

static void
wrgtk_tray_icon_get_padding_property (WRGtkTrayIcon *icon)
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
			       icon->priv->padding_atom,
			       0, G_MAXLONG, FALSE,
			       XA_CARDINAL,
			       &type, &format, &nitems,
			       &bytes_after, &(prop.prop_ch));
  error = gdk_error_trap_pop ();

  if (!error && result == Success &&
      type == XA_CARDINAL && nitems == 1 && format == 32)
    {
      gint padding;

      padding = prop.prop[0];

      if (icon->priv->padding != padding)
	{
	  icon->priv->padding = padding;

	  g_object_notify (G_OBJECT (icon), "padding");
	}
    }

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
      else if (xev->xany.type == PropertyNotify &&
               xev->xproperty.atom == icon->priv->colors_atom)
        {
          GTK_NOTE (PLUGSOCKET,
                    g_print ("GtkStatusIcon %p: got PropertyNotify on manager window for colors atom\n", icon));

          wrgtk_tray_icon_get_colors_property (icon);
        }
      else if (xev->xany.type == PropertyNotify &&
               xev->xproperty.atom == icon->priv->padding_atom)
        {
          wrgtk_tray_icon_get_padding_property (icon);
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
  GtkWidget *widget;
  XClientMessageEvent ev;
  Display *display;

  widget = GTK_WIDGET (icon);

  memset (&ev, 0, sizeof (ev));
  ev.type = ClientMessage;
  ev.window = window;
  ev.message_type = icon->priv->system_tray_opcode_atom;
  ev.format = 32;
  ev.data.l[0] = gdk_x11_get_server_time (gtk_widget_get_window (widget));
  ev.data.l[1] = message;
  ev.data.l[2] = data1;
  ev.data.l[3] = data2;
  ev.data.l[4] = data3;

  display = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (widget));

  gdk_error_trap_push ();
  XSendEvent (display,
	      icon->priv->manager_window, False, NoEventMask, (XEvent *)&ev);
  gdk_error_trap_pop_ignored ();
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

      gdkwin = gdk_x11_window_lookup_for_display (display,
                                                  icon->priv->manager_window);

      gdk_window_add_filter (gdkwin, wrgtk_tray_icon_manager_filter, icon);

      wrgtk_tray_icon_get_orientation_property (icon);
      wrgtk_tray_icon_get_visual_property (icon);
      wrgtk_tray_icon_get_colors_property (icon);
      wrgtk_tray_icon_get_padding_property (icon);

      if (gtk_widget_get_realized (GTK_WIDGET (icon)))
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
#ifdef G_ENABLE_DEBUG
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);
#endif

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
wrgtk_tray_icon_set_visual (WRGtkTrayIcon *icon)
{
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (icon));
  GdkVisual *visual = icon->priv->manager_visual;

  /* To avoid uncertainty about colormaps, _NET_SYSTEM_TRAY_VISUAL is supposed
   * to be either the screen default visual or a TrueColor visual; ignore it
   * if it is something else
   */
  if (visual && gdk_visual_get_visual_type (visual) != GDK_VISUAL_TRUE_COLOR)
    visual = NULL;

  if (visual == NULL)
    visual = gdk_screen_get_system_visual (screen);

  gtk_widget_set_visual (GTK_WIDGET (icon), visual);
}

static void
wrgtk_tray_icon_realize (GtkWidget *widget)
{
  WRGtkTrayIcon *icon = GTK_TRAY_ICON (widget);
  GdkWindow *window;

  /* Set our visual before realizing */
  wrgtk_tray_icon_set_visual (icon);

  GTK_WIDGET_CLASS (wrgtk_tray_icon_parent_class)->realize (widget);
  window = gtk_widget_get_window (widget);
  if (icon->priv->manager_visual_rgba)
    {
      /* Set a transparent background */
      GdkColor transparent = { 0, 0, 0, 0 }; /* Only pixel=0 matters */
      gdk_window_set_background (window, &transparent);
    }
  else
    {
      /* Set a parent-relative background pixmap */
      gdk_window_set_background_pattern (window, NULL);
    }

  GTK_NOTE (PLUGSOCKET,
	    g_print ("GtkStatusIcon %p: realized, window: %lx, socket window: %lx\n",
		     widget,
		     (gulong) GDK_WINDOW_XID (window),
		     gtk_plug_get_socket_window (GTK_PLUG (icon)) ?
		     (gulong) GDK_WINDOW_XID (gtk_plug_get_socket_window (GTK_PLUG (icon))) : 0UL));

  if (icon->priv->manager_window != None)
    wrgtk_tray_icon_send_dock_request (icon);
}

static void
wrgtk_tray_icon_style_updated (GtkWidget   *widget)
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
  Display *xdisplay;
 
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
  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (GTK_WIDGET (icon)));
  gdk_error_trap_push ();
  while (len > 0)
    {
      XClientMessageEvent ev;

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
    }
  gdk_error_trap_pop_ignored ();

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
_wrgtk_tray_icon_new_for_screen (GdkScreen  *screen, 
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

gint
_wrgtk_tray_icon_get_padding (WRGtkTrayIcon *icon)
{
  g_return_val_if_fail (GTK_IS_TRAY_ICON (icon), 0);

  return icon->priv->padding;
}
