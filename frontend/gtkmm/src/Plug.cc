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


#ifdef HAVE_GTK3

bool
Plug::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  cr->set_source_rgba(0, 0, 0, 0);
  cr->set_operator(Cairo::OPERATOR_SOURCE);
  cr->paint();

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
  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(gobj()));
  GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(gobj()));

  if (gdk_screen_get_rgba_visual(screen) != NULL && gdk_display_supports_composite(display))
    {
      GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual);
    }

  Gtk::Widget::on_realize();
}

#endif
