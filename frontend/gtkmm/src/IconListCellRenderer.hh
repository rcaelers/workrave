// IconListCellRenderer.hh --- Notebook like widget cell renderer
//
// Copyright (C) 2003, 2007, 2011 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef ICON_LIST_CELL_RENDERER_HH
#define ICON_LIST_CELL_RENDERER_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm/cellrenderer.h>
#include <gtkmm/cellrendererpixbuf.h>
#include <gtkmm/cellrenderertext.h>

class IconListCellRenderer : public Gtk::CellRenderer
{
public:
  IconListCellRenderer();
  virtual ~IconListCellRenderer();

  Glib::PropertyProxy<Glib::ustring> property_text();
  Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf();

protected:
#ifdef HAVE_GTK3
  virtual void get_preferred_width_vfunc(Gtk::Widget &widget, int &minimum_width, int &natural_width) const;
  virtual void get_preferred_height_for_width_vfunc(Gtk::Widget &widget, int width, int &minimum_height, int &natural_height) const;
  virtual void get_preferred_height_vfunc(Gtk::Widget &widget, int &minimum_height, int &natural_height) const;
  virtual void get_preferred_width_for_height_vfunc(Gtk::Widget &widget, int height, int &minimum_width, int &natural_width) const;
  virtual void render_vfunc(const Cairo::RefPtr<Cairo::Context> &cr, Gtk::Widget &widget, const Gdk::Rectangle &background_area, const Gdk::Rectangle &cell_area, Gtk::CellRendererState flags);

#else  
  virtual void get_size_vfunc(Gtk::Widget &widget,
                              const Gdk::Rectangle *cell_area,
                              int* x_offset, int* y_offset,
                              int* width,    int* height) const;

  virtual void get_size_vfunc(Gtk::Widget &widget,
                              const Gdk::Rectangle *cell_area,
                              int* x_offset, int* y_offset,
                              int* width,    int* height);

  virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable> &window,
                            Gtk::Widget &widget,
                            const Gdk::Rectangle &background_area,
                            const Gdk::Rectangle &cell_area,
                            const Gdk::Rectangle &expose_area,
                            Gtk::CellRendererState flags);
#endif
  
private:
  void update_properties();

  Gtk::CellRendererPixbuf pixbuf_renderer;
  Gtk::CellRendererText text_renderer;

  Glib::Property<Glib::ustring> property_text_;
  Glib::Property<Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf_;
};



#endif // ICON_LIST_CELL_RENDERER_HH
