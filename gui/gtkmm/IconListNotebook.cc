#include "IconListNotebook.hh"

IconListNotebook::IconListNotebook()
{
  icon_list.show();
  icon_list.set_headers_visible(false);
  Gtk::ScrolledWindow *scroller = manage(new Gtk::ScrolledWindow());
  scroller->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
  scroller->set_shadow_type(Gtk::SHADOW_IN);
  scroller->show();
  scroller->add(icon_list);

  Gtk::TreeModelColumn< Glib::ustring > col_title;
  Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > col_img;
  Gtk::TreeModelColumn< Glib::RefPtr<Gtk::Widget> > col_page;
  column_record.add(col_title);
  column_record.add(col_img);
  column_record.add(col_page);
  list_store = Gtk::ListStore::create(column_record);
  icon_list.set_model(list_store);

  Gtk::TreeViewColumn *tvc = manage(new Gtk::TreeViewColumn("Bla"));
  tvc->add_attribute(, col_title);
  tvc
  
//195           	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (dlg->priv->notebook), FALSE);
//196           	gtk_notebook_set_show_border (GTK_NOTEBOOK (dlg->priv->notebook), 
//197           				      FALSE);
//
//198 campd 1.2 	gtk_container_set_border_width (GTK_CONTAINER (dlg->priv->notebook),  

  pack_start(*scroller, false, false, 0);
  pack_start(notebook, true, true, 0);
}


void
IconListNotebook::add_page(const char *name, Gtk::Image &img,
                           Gtk::Widget &widget)
{
  notebook.pages().push_back(Gtk::Notebook_Helpers::TabElem
                             (widget, name));
}
