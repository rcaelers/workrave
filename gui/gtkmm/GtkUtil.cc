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
  Gtk::manage(img);
  btn->add(*img);
  return btn;
}

Gtk::Widget *
GtkUtil::create_label_with_icon(const char *text, const char *icon)
{
  Gtk::HBox *box = new Gtk::HBox(false, 3);
  Gtk::Label *lab = Gtk::manage(new Gtk::Label(text));
  Gtk::Image *img = Gtk::manage(new Gtk::Image(icon));
  box->pack_start(*img, false, false, 0);
  box->pack_start(*lab, false, false, 0);
  return box;
}


Gtk::Widget *
GtkUtil::create_label_for_break(GUIControl::BreakId id)
{
  GUIControl *guic = GUIControl::get_instance();
  GUIControl::TimerData *timer = guic->get_timer_data(id);
#if 1
  timer = &(guic->timers[(int)id]);
#endif  
  Gtk::Widget *label = 
    GtkUtil::create_label_with_icon (timer->label, timer->icon.c_str());
  return label;
}
