// Hig.hh --- Gnome HIG stuff
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

#ifndef HIG_HH
#define HIG_HH

#include <gtkmm.h>

class HigDialog : public Gtk::Dialog
{
public:
  HigDialog();
  HigDialog(const Glib::ustring& title, bool modal=false,
            bool use_separator=false);
  void add_panel(Gtk::Widget &panel, bool expand, bool fill);
  
private:
  void set_hig_defaults();
};

class HigCategoryPanel : public Gtk::VBox
{
public:
  HigCategoryPanel(const char *lab);
  void add(const char *lab, Gtk::Widget &widget);
  void add(Gtk::Widget &widget);

private:
  Gtk::VBox *options_box;
  Glib::RefPtr<Gtk::SizeGroup> size_group;
};

class HigCategoriesPanel : public Gtk::VBox
{
public:
  HigCategoriesPanel();
  void add(Gtk::Widget &panel);
};


#endif // HIG_HH
