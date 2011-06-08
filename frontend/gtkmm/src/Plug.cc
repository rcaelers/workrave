// Plug.cc --- Time Bar
//
// Copyright (C) 2002 - 2009, 2011 Rob Caelers & Raymond Penners
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

#include "preinclude.h"
#include "debug.hh"

#include <stdio.h>
#include <stdlib.h>

#include "Plug.hh"

#include <gtkmm.h>
#include <gdkmm.h>

#include <gdk/gdkx.h>
#include <X11/Xatom.h>

using namespace std;

//! Constructor
Plug::Plug() 
{
  add_events(Gdk::EXPOSURE_MASK);
}


//! Constructor
Plug::Plug(int id) : Gtk::Plug(id)
{
  add_events(Gdk::EXPOSURE_MASK);
}

//! Destructor
Plug::~Plug()
{
}


bool
Plug::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  TRACE_ENTER("Plug::on_draw");

  cr->set_source_rgba(0, 0, 255, 0);
  cr->set_operator(Cairo::OPERATOR_SOURCE);
  cr->paint();

  // GtkTrayIcon *icon = GTK_TRAY_ICON (widget);
  // GtkWidget *focus_child;
  // GdkWindow *window;
  // gint border_width;
  // gboolean retval = FALSE;
  // cairo_surface_t *target;

  // window = gtk_widget_get_window (widget);
  // target = cairo_get_group_target (cr);

  // if (icon->priv->manager_visual_rgba ||
  //     cairo_surface_get_type (target) != CAIRO_SURFACE_TYPE_XLIB ||
  //     cairo_xlib_surface_get_drawable (target) != GDK_WINDOW_XID (window))
  //   {
      /* Clear to transparent */
      // cairo_set_source_rgba (cr, 0, 0, 0, 0);
      // cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      // cairo_paint (cr);
  //   }
  // else
  //   {
  //     GdkRectangle clip;

  //     if (gdk_cairo_get_clip_rectangle (cr, &clip))
  //       {
  //         /* Clear to parent-relative pixmap
  //          * We need to use direct X access here because GDK doesn't know about
  //          * the parent realtive pixmap. */
  //         cairo_surface_flush (target);

  //         XClearArea (GDK_WINDOW_XDISPLAY (window),
  //                     GDK_WINDOW_XID (window),
  //                     clip.x, clip.y,
  //                     clip.width, clip.height,
  //                     False);
  //         cairo_surface_mark_dirty_rectangle (target, 
  //                                             clip.x, clip.y,
  //                                             clip.width, clip.height);
  //       }
  //   }
  
  TRACE_EXIT();
  return Gtk::Widget::on_draw(cr);
}

void
Plug::on_unrealize()
{
  Gtk::Widget::on_unrealize();
}

void
Plug::on_realize()
{
  TRACE_ENTER("Plug::on_realize");

  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(gobj()));
  GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(gobj()));
 
  if (gdk_screen_get_rgba_visual(screen) != NULL && gdk_display_supports_composite(display))
    {
      GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

      TRACE_MSG("set visual");
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual);

      int red_prec, green_prec, blue_prec, depth;
      gdk_visual_get_red_pixel_details (visual, NULL, NULL, &red_prec);
      gdk_visual_get_green_pixel_details (visual, NULL, NULL, &green_prec);
      gdk_visual_get_blue_pixel_details (visual, NULL, NULL, &blue_prec);
      depth = gdk_visual_get_depth (visual);

      TRACE_MSG(red_prec << " " << green_prec << " " << blue_prec << " " << depth);

      GdkVisual *v2 = gtk_widget_get_visual(GTK_WIDGET(gobj()));


      TRACE_MSG( ((long long)visual) << " " << ((long long) v2));
    }

  Gtk::Widget::on_realize();

  GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(gobj()));

  /* Set a transparent background */
  GdkColor transparent = { 0, 0, 0, 0 }; /* Only pixel=0 matters */
  gdk_window_set_background(window, &transparent);
  
  TRACE_EXIT();
}
