// GtkUtil.cc --- Gtk utilities
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

#include "GtkUtil.hh"


Gtk::Button *
GtkUtil::create_stock_button_without_text(const Gtk::StockID& stock_id)
{
  Gtk::Button *btn = new Gtk::Button();
  Gtk::Image *img = new Gtk::Image(stock_id, 
                                   Gtk::ICON_SIZE_BUTTON);
  // FIXME: manage(img) ??
  btn->add(*img);
  return btn;
}

