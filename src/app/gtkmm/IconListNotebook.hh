// IconListNotebook.hh --- Notebook like widget 
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

#ifndef ICON_LIST_NOTEBOOK_HH
#define ICON_LIST_NOTEBOOK_HH

#include <gtkmm/box.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/notebook.h>
#include <gtkmm/liststore.h>

class IconListNotebook : public Gtk::HBox
{
public:
  IconListNotebook();
  void add_page(const char *name, Glib::RefPtr<Gdk::Pixbuf>, Gtk::Widget &widget);

private:
  void on_page_changed();
  
  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> text;
    Gtk::TreeModelColumn<Gtk::Widget*> page;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > pixbuf;
    
    ModelColumns() { add(text); add(page); add(pixbuf); }
  };
  
  Gtk::Notebook notebook;
  Gtk::TreeView icon_list;
  Glib::RefPtr<Gtk::ListStore> list_store;
  ModelColumns model_columns;
};



#endif // ICON_LIST_NOTEBOOK_HH
