// IconListCellRenderer.hh --- Notebook like widget cell renderer 
//
// Copyright (C) 2003 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$
//

#ifndef ICON_LIST_CELL_RENDERER_HH
#define ICON_LIST_CELL_RENDERER_HH

class IconListCellRenderer : public Gtk::CellRenderer
{
public:
  IconListCellRenderer();
  virtual ~IconListCellRenderer();

  Glib::PropertyProxy<Glib::ustring> property_text();
  Glib::PropertyProxy<Glib::RefPtr<Gdk::Pixbuf> > property_pixbuf();

protected:
  virtual void get_size_vfunc(Gtk::Widget& widget,
                              const Gdk::Rectangle *cell_area,
                              int* x_offset, int* y_offset,
                              int* width,    int* height);

  virtual void render_vfunc(const Glib::RefPtr<Gdk::Window>& window,
                            Gtk::Widget& widget,
                            const Gdk::Rectangle& background_area,
                            const Gdk::Rectangle& cell_area,
                            const Gdk::Rectangle& expose_area,
                            Gtk::CellRendererState flags);


private:
  Gtk::CellRendererPixbuf pixbuf_renderer;
  Gtk::CellRendererText text_renderer;
};



#endif // ICON_LIST_CELL_RENDERER_HH
