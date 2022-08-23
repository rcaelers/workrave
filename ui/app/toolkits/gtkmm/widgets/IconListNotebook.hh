// Copyright (C) 2003, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef ICON_LIST_NOTEBOOK_HH
#define ICON_LIST_NOTEBOOK_HH

#include <gtkmm/box.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/notebook.h>
#include <gtkmm/liststore.h>

class IconListNotebook : public GtkCompat::Box
{
public:
  IconListNotebook();
  void add_page(const char *name, Glib::RefPtr<Gdk::Pixbuf>, Gtk::Widget &widget);

private:
  void on_page_changed();

  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> text;
    Gtk::TreeModelColumn<Gtk::Widget *> page;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> pixbuf;

    ModelColumns()
    {
      add(text);
      add(page);
      add(pixbuf);
    }
  };

  Gtk::Notebook notebook;
  Gtk::TreeView icon_list;
  Glib::RefPtr<Gtk::ListStore> list_store;
  ModelColumns model_columns;
};

#endif // ICON_LIST_NOTEBOOK_HH
