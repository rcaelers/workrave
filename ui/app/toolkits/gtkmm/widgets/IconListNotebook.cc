// Copyright (C) 2003 - 2011 Raymond Penners <raymond@dotsphinx.com>
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

#include "debug.hh"
#include "commonui/nls.h"

#include <gtkmm.h>
#include <gtk/gtk.h>

#include "IconListNotebook.hh"
#include "IconListCellRenderer.hh"

IconListNotebook::IconListNotebook()
  : GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6)
{
  icon_list.show();
  icon_list.set_headers_visible(false);
  Gtk::ScrolledWindow *scroller = Gtk::manage(new Gtk::ScrolledWindow());
  scroller->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  scroller->set_shadow_type(Gtk::SHADOW_IN);
  scroller->show();
  scroller->add(icon_list);

  list_store = Gtk::ListStore::create(model_columns);
  icon_list.set_model(list_store);

  IconListCellRenderer *renderer = new IconListCellRenderer();
  Gtk::TreeViewColumn *tvc = new Gtk::TreeViewColumn("Bla", *Gtk::manage(renderer));
  icon_list.append_column(*Gtk::manage(tvc));
  tvc->add_attribute(renderer->property_text(), model_columns.text);
  tvc->add_attribute(renderer->property_pixbuf(), model_columns.pixbuf);

  Glib::RefPtr<Gtk::TreeSelection> selection = icon_list.get_selection();
  selection->set_mode(Gtk::SELECTION_SINGLE);
  selection->signal_changed().connect(sigc::mem_fun(*this, &IconListNotebook::on_page_changed));

  notebook.set_show_tabs(false);
  notebook.set_show_border(false);

  pack_start(*scroller, false, false, 0);
  pack_start(notebook, true, true, 0);
}

void
IconListNotebook::on_page_changed()
{
  if (const Gtk::TreeModel::iterator selected = icon_list.get_selection()->get_selected())
    {
      Gtk::Widget *page = (*selected)[model_columns.page];
      int page_num = gtk_notebook_page_num(notebook.gobj(), page->gobj());
      notebook.set_current_page(page_num);
    }
}

void
IconListNotebook::add_page(const char *name, Glib::RefPtr<Gdk::Pixbuf> pixbuf, Gtk::Widget &widget)
{
  notebook.append_page(widget, name);
  Gtk::TreeRow row = *list_store->append();
  row[model_columns.text] = Glib::ustring(name);
  row[model_columns.pixbuf] = pixbuf;
  row[model_columns.page] = &widget;
}
