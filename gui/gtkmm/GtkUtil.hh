// GtkUtil.hh --- Gtk utilities
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

#ifndef GTKUTIL_HH
#define GTKUTIL_HH

#include <gtkmm.h>
#include "GUIControl.hh"

class GtkUtil
{
public:
  static Gtk::Button *
  create_stock_button_without_text(const Gtk::StockID& stock_id);

  static Gtk::Widget *
  create_label_with_icon(const char *text, const char *icon);

  static Gtk::Widget *
  create_label_for_break(GUIControl::BreakId id);
};

#endif // GTKMMGUI_HH
