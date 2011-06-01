// IconListCellRenderer.cc --- Icon list cell renderer
//
// Copyright (C) 2001, 2002, 2003, 2007, 2011 Rob Caelers & Raymond Penners
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

#include <gtk/gtk.h>
#include <gtkmm.h>

#include "debug.hh"

#include "IconListCellRenderer.hh"

#define PAD 3
#define SPACE 5

IconListCellRenderer::IconListCellRenderer()
  : Glib::ObjectBase (typeid(IconListCellRenderer)),
    Gtk::CellRenderer(),
    property_text_   (*this, "text"),
    property_pixbuf_ (*this, "pixbuf")
{
  update_properties();
}

IconListCellRenderer::~IconListCellRenderer()
{}

Glib::PropertyProxy<Glib::ustring> IconListCellRenderer::property_text()
{
  return property_text_.get_proxy();
}

Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > IconListCellRenderer::property_pixbuf()
{
  return property_pixbuf_.get_proxy();
}

void
IconListCellRenderer::update_properties()
{
  text_renderer.property_text() = property_text_;
  pixbuf_renderer.property_pixbuf() = property_pixbuf_;
}

#ifdef HAVE_GTK3

void
IconListCellRenderer::get_preferred_width_vfunc(Gtk::Widget &widget, int &minimum_width, int &natural_width) const
{
  TRACE_ENTER("IconListCellRenderer::get_preferred_width_vfunc");
  int text_minimum_width, text_natural_width;
  int pixbuf_minimum_width, pixbuf_natural_width;

  // FIXME:
  const_cast<IconListCellRenderer*>(this)->update_properties();
  
  text_renderer.get_preferred_width(widget, text_minimum_width, text_natural_width);
  pixbuf_renderer.get_preferred_width(widget, pixbuf_minimum_width, pixbuf_natural_width);

  minimum_width = MAX(text_minimum_width, pixbuf_minimum_width) + 2 * SPACE;
  natural_width = MAX(text_natural_width, pixbuf_natural_width) + 2 * SPACE;
  TRACE_MSG(minimum_width << " " << natural_width);
  TRACE_EXIT();
}

void IconListCellRenderer::get_preferred_height_for_width_vfunc(Gtk::Widget &widget, int width, int &minimum_height, int &natural_height) const
{
  (void) width;
  TRACE_ENTER("IconListCellRenderer::get_preferred_height_for_width_vfunc");
  get_preferred_height_vfunc(widget, minimum_height, natural_height);
  TRACE_MSG(minimum_height << " " << natural_height);
  TRACE_EXIT();

}
  
void IconListCellRenderer::get_preferred_height_vfunc(Gtk::Widget &widget, int &minimum_height, int &natural_height) const
{
  TRACE_ENTER("IconListCellRenderer::get_preferred_height_vfunc");
  int text_minimum_height, text_natural_height;
  int pixbuf_minimum_height, pixbuf_natural_height;

  // FIXME:
  const_cast<IconListCellRenderer*>(this)->update_properties();
  
  text_renderer.get_preferred_height(widget, text_minimum_height, text_natural_height);
  pixbuf_renderer.get_preferred_height(widget, pixbuf_minimum_height, pixbuf_natural_height);

  minimum_height = text_minimum_height + pixbuf_minimum_height + PAD;
  natural_height = text_natural_height + pixbuf_natural_height + PAD;

  TRACE_MSG(minimum_height << " " << natural_height);
  TRACE_EXIT();
}

void IconListCellRenderer::get_preferred_width_for_height_vfunc(Gtk::Widget &widget, int height, int &minimum_width, int &natural_width) const
{
  (void) height;
  TRACE_ENTER("IconListCellRenderer::get_preferred_width_for_height_vfunc");
  get_preferred_width_vfunc(widget, minimum_width, natural_width);
  TRACE_MSG(minimum_width << " " << natural_width);
  TRACE_EXIT();
}

void IconListCellRenderer::render_vfunc(const Cairo::RefPtr<Cairo::Context> &cr, Gtk::Widget &widget, const Gdk::Rectangle &background_area, const Gdk::Rectangle &cell_area, Gtk::CellRendererState flags)
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

#else
   
void
IconListCellRenderer::get_size_vfunc(Gtk::Widget& widget,
                                     const Gdk::Rectangle* cell_area,
                                     int* x_offset, int* y_offset,
                                     int* width,    int* height) const
{
  // FIXME: gtkmm 2.4 changed their API. this method now needs to be const.
  //        This is a (temporary) hack to get thing working again.
  IconListCellRenderer *r = const_cast<IconListCellRenderer *>(this);
  r->get_size_vfunc(widget, cell_area, x_offset, y_offset, width, height);
}

void
IconListCellRenderer::get_size_vfunc(Gtk::Widget& widget,
                                     const Gdk::Rectangle* cell_area,
                                     int* x_offset, int* y_offset,
                                     int* width,    int* height)
{
  (void) x_offset;
  (void) y_offset;

  int text_x_offset;
  int text_y_offset;
  int text_width;
  int text_height;

  update_properties();
  GdkRectangle *rect = NULL;
  if (cell_area)
    {
      rect = (GdkRectangle *) cell_area->gobj();
    }

  GtkCellRenderer *rend = GTK_CELL_RENDERER(text_renderer.gobj());
  gtk_cell_renderer_get_size (rend, widget.gobj(), rect,
                              NULL, NULL, width, height);

  rend = GTK_CELL_RENDERER(pixbuf_renderer.gobj());
  gtk_cell_renderer_get_size (rend, widget.gobj(),
                              rect,
                              &text_x_offset, &text_y_offset,
                              &text_width, &text_height);
  if (height) {
    *height = *height + text_height + PAD;
  }

  if (width) {
    *width = MAX (*width, text_width);
    *width += SPACE * 2;
  }
}


void
IconListCellRenderer::render_vfunc(
                                   const Glib::RefPtr<Gdk::Drawable>& window,
                                   Gtk::Widget& widget,
                                   const Gdk::Rectangle& bg_area,
                                   const Gdk::Rectangle& cell_area,
                                   const Gdk::Rectangle& expose_area,
                                   Gtk::CellRendererState flags)
{
  update_properties();

  GdkRectangle text_area;
  GdkRectangle pixbuf_area;
  int width, height;

  GdkRectangle *ca = (GdkRectangle *) cell_area.gobj();
  GtkWidget *widg = widget.gobj();
  GdkWindow *wind = window->gobj();
  GtkCellRenderer *prend = GTK_CELL_RENDERER(pixbuf_renderer.gobj());
  gtk_cell_renderer_get_size (prend, widg, ca,
                              NULL, NULL, &width, &height);
    
  pixbuf_area.y = ca->y;
  pixbuf_area.x = ca->x;
  pixbuf_area.height = height;
  pixbuf_area.width = ca->width;

  GtkCellRenderer *trend = GTK_CELL_RENDERER(text_renderer.gobj());
  gtk_cell_renderer_get_size (trend, widg, ca,
                              NULL, NULL, &width, &height);

  text_area.x = ca->x + (ca->width - width) / 2;
  text_area.y = ca->y + (pixbuf_area.height + PAD);
  text_area.height = height;
  text_area.width = width;

  gtk_cell_renderer_render (prend, wind, widg,
                            (GdkRectangle*)bg_area.gobj(), &pixbuf_area,
                            (GdkRectangle*)expose_area.gobj(),
                            (GtkCellRendererState) flags);

  gtk_cell_renderer_render (trend, wind, widg,
                            (GdkRectangle*)bg_area.gobj(),
                            &text_area,
                            (GdkRectangle*)expose_area.gobj(),
                            (GtkCellRendererState)  flags);
}

#endif
