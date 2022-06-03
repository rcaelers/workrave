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
#  include "config.h"
#endif

#include <gtk/gtk.h>
#include <gtkmm.h>

#include "IconListCellRenderer.hh"

#define PAD 3
#define SPACE 5

IconListCellRenderer::IconListCellRenderer()
  : Glib::ObjectBase(typeid(IconListCellRenderer))
  , Gtk::CellRenderer()
  , property_text_(*this, "text")
  , property_pixbuf_(*this, "pixbuf")
{
  update_properties();
}

Glib::PropertyProxy<Glib::ustring>
IconListCellRenderer::property_text()
{
  return property_text_.get_proxy();
}

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf>>
IconListCellRenderer::property_pixbuf()
{
  return property_pixbuf_.get_proxy();
}

void
IconListCellRenderer::update_properties()
{
  text_renderer.property_text() = property_text_;
  pixbuf_renderer.property_pixbuf() = property_pixbuf_;
}

void
IconListCellRenderer::get_preferred_width_vfunc(Gtk::Widget &widget, int &minimum_width, int &natural_width) const
{
  int text_minimum_width, text_natural_width;
  int pixbuf_minimum_width, pixbuf_natural_width;

  const_cast<IconListCellRenderer *>(this)->update_properties();

  text_renderer.get_preferred_width(widget, text_minimum_width, text_natural_width);
  pixbuf_renderer.get_preferred_width(widget, pixbuf_minimum_width, pixbuf_natural_width);

  minimum_width = MAX(text_minimum_width, pixbuf_minimum_width) + 2 * SPACE;
  natural_width = MAX(text_natural_width, pixbuf_natural_width) + 2 * SPACE;
}

void
IconListCellRenderer::get_preferred_height_for_width_vfunc(Gtk::Widget &widget,
                                                           int width,
                                                           int &minimum_height,
                                                           int &natural_height) const
{
  (void)width;
  get_preferred_height_vfunc(widget, minimum_height, natural_height);
}

void
IconListCellRenderer::get_preferred_height_vfunc(Gtk::Widget &widget, int &minimum_height, int &natural_height) const
{
  int text_minimum_height, text_natural_height;
  int pixbuf_minimum_height, pixbuf_natural_height;

  const_cast<IconListCellRenderer *>(this)->update_properties();

  text_renderer.get_preferred_height(widget, text_minimum_height, text_natural_height);
  pixbuf_renderer.get_preferred_height(widget, pixbuf_minimum_height, pixbuf_natural_height);

  minimum_height = text_minimum_height + pixbuf_minimum_height + PAD;
  natural_height = text_natural_height + pixbuf_natural_height + PAD;
}

void
IconListCellRenderer::get_preferred_width_for_height_vfunc(Gtk::Widget &widget,
                                                           int height,
                                                           int &minimum_width,
                                                           int &natural_width) const
{
  (void)height;
  get_preferred_width_vfunc(widget, minimum_width, natural_width);
}

void
IconListCellRenderer::render_vfunc(const Cairo::RefPtr<Cairo::Context> &cr,
                                   Gtk::Widget &widget,
                                   const Gdk::Rectangle &background_area,
                                   const Gdk::Rectangle &cell_area,
                                   Gtk::CellRendererState flags)
{
  update_properties();

  Gdk::Rectangle text_area;
  Gdk::Rectangle pixbuf_area;

  Gtk::Requisition minimum_size, natural_size;

  pixbuf_renderer.get_preferred_size(widget, minimum_size, natural_size);

  pixbuf_area.set_x(cell_area.get_x());
  pixbuf_area.set_y(cell_area.get_y());
  pixbuf_area.set_width(cell_area.get_width());
  pixbuf_area.set_height(natural_size.height);

  text_renderer.get_preferred_size(widget, minimum_size, natural_size);

  text_area.set_x(cell_area.get_x() + (cell_area.get_width() - natural_size.width) / 2);
  text_area.set_y(cell_area.get_y() + (pixbuf_area.get_height() + PAD));
  text_area.set_height(natural_size.height);
  text_area.set_width(natural_size.width);

  pixbuf_renderer.render(cr, widget, background_area, pixbuf_area, flags);
  text_renderer.render(cr, widget, background_area, text_area, flags);
}
