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

#include "IconListNotebook.hh"
#include "IconListCellRenderer.hh"



IconListNotebook::IconListNotebook()
{
  icon_list.show();
  icon_list.set_headers_visible(false);
  Gtk::ScrolledWindow *scroller = manage(new Gtk::ScrolledWindow());
  scroller->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  scroller->set_shadow_type(Gtk::SHADOW_IN);
  scroller->show();
  scroller->add(icon_list);

  list_store = Gtk::ListStore::create(model_columns);
  icon_list.set_model(list_store);

  IconListCellRenderer *renderer = manage(new IconListCellRenderer());
  Gtk::TreeViewColumn *tvc = manage(new Gtk::TreeViewColumn("Bla",
                                                            *renderer));
  tvc->add_attribute(renderer->property_text(),
                     model_columns.text);
  tvc->add_attribute(renderer->property_pixbuf(),
                     model_columns.pixbuf);

  icon_list.append_column(*tvc);
  
//195           	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (dlg->priv->notebook), FALSE);
//196           	gtk_notebook_set_show_border (GTK_NOTEBOOK (dlg->priv->notebook), 
//197           				      FALSE);
//
//198 campd 1.2 	gtk_container_set_border_width (GTK_CONTAINER (dlg->priv->notebook),  

  pack_start(*scroller, false, false, 0);
  pack_start(notebook, true, true, 0);
}


void
IconListNotebook::add_page(const char *name, Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                           Gtk::Widget &widget)
{
  notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem
                             (widget, name));
  Gtk::TreeRow row = *list_store->append();
  row[model_columns.text] = Glib::ustring(name);
  row[model_columns.pixbuf] = pixbuf;
}
