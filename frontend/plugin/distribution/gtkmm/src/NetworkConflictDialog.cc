// NetworkConflictDialog.cc --- NetworkConflict dialog
//
// Copyright (C) 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
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
// $Id: NetworkConflictDialog.cc 1301 2007-08-30 21:25:47Z rcaelers $

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#ifdef HAVE_UNISTD
#include <unistd.h>
#endif

#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/stock.h>

#include "NetworkConflictDialog.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "Util.hh"
#include "GtkUtil.hh"

using namespace workrave;

NetworkConflictDialog::NetworkConflictDialog()
  : HigDialog(_("Configuration conflict"), true, false)
{
  TRACE_ENTER("NetworkConflictDialog::NetworkConflictDialog");

  set_default_size(600, 400);

  create_model();
  
  create_ui();

//   Gtk::HBox *box = manage(new Gtk::HBox(false, 6));
//   box->pack_start(*ui, true, true, 0);

  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();

  TRACE_EXIT();
}


//! Destructor.
NetworkConflictDialog::~NetworkConflictDialog()
{
  TRACE_ENTER("NetworkConflictDialog::~NetworkConflictDialog");
  TRACE_EXIT();
}


void
NetworkConflictDialog::init()
{
//   ICore *core = CoreFactory::get_core();
}


void
NetworkConflictDialog::create_ui()
{
  pixbuf_left_arrow =
    Gdk::Pixbuf::create_from_file("/usr/share/icons/Tango/32x32/actions/stock_left.png");
  pixbuf_right_arrow =
    Gdk::Pixbuf::create_from_file("/usr/share/icons/Tango/32x32/actions/stock_right.png");

  // Info text
  const char *label =
    _("A connection has been established with a remote Workrave.\n"
      "This Workrave uses different break setting. Please resolve..");
  Gtk::Label *info_lab = Gtk::manage(new Gtk::Label(label));
  info_lab->set_alignment(0.0);
  get_vbox()->pack_start(*info_lab, false, false, 0);


  Glib::RefPtr<Gtk::SizeGroup> size_group_lab;  
  size_group_lab = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);

  Glib::RefPtr<Gtk::SizeGroup> size_group_col;  
  size_group_col = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  
  Gtk::Label *tim_lab = manage(GtkUtil::create_label(std::string("Micro-break"), true));
  tim_lab->set_alignment(0.0);
  get_vbox()->pack_start(*tim_lab, false, false, 0);

  Gtk::HBox *ibox = manage(new Gtk::HBox());
  get_vbox()->pack_start(*ibox, false, false, 0);

  Gtk::Label *indent_lab = Gtk::manage(new Gtk::Label("    "));
  ibox->pack_start(*indent_lab, false, false, 0);
  Gtk::VBox *options_box = manage(new Gtk::VBox());
  ibox->pack_start(*options_box, false, false, 0);
  options_box->set_spacing(6);

  Gtk::Label *lab;
  Gtk::Label *loclab;
  Gtk::Label *remlab;

  lab = manage(GtkUtil::create_label(std::string("Micro-break"), false));
  lab->set_alignment(0.0);

  loclab = manage(GtkUtil::create_label(std::string("Local value"), false));
  loclab->set_alignment(0.0);

  remlab = manage(GtkUtil::create_label(std::string("Remote value"), false));
  remlab->set_alignment(0.0);
  
  size_group_lab->add_widget(*lab);
  Gtk::HBox *box = manage(new Gtk::HBox());
  box->set_spacing(6);
  box->pack_start(*lab, false, true, 0);
  box->pack_start(*loclab, false, false, 0);
  box->pack_start(*remlab, false, false, 0);
  options_box->pack_start(*box, false, false, 0);
  
  
// //   //
// //   Gtk::HBox *hbox = manage(new Gtk::HBox(false, 6));

// //   conflict_list = manage(new Gtk::TreeView());
// //   // create_model();

//   // create tree view
//   conflict_list->set_model(conflict_store);
//   conflict_list->set_rules_hint();
//   conflict_list->set_search_column(conflict_columns.name.index());

//   Glib::RefPtr<Gtk::TreeSelection> selection = conflict_list->get_selection();
//   selection->set_mode(Gtk::SELECTION_MULTIPLE);

//   Gtk::CellRendererText *renderer = NULL;
//   Gtk::CellRendererPixbuf *rendererpix = NULL;
//   Gtk::TreeViewColumn *column = NULL;
//   int cols_count = 0;

//   renderer = Gtk::manage(new Gtk::CellRendererText());
//   cols_count = conflict_list->append_column(_("Setting"), *renderer);
//   column = conflict_list->get_column(cols_count-1);
//   column->add_attribute(renderer->property_text(), conflict_columns.name);
//   column->set_resizable(true);
//   renderer->property_editable().set_value(false);

//   renderer = Gtk::manage(new Gtk::CellRendererText());
//   cols_count = conflict_list->append_column(_("Local value"), *renderer);
//   column = conflict_list->get_column(cols_count-1);
//   column->add_attribute(renderer->property_text(), conflict_columns.local_value);
//   column->set_resizable(true);
//   renderer->property_editable().set_value(false);

//   rendererpix = Gtk::manage(new Gtk::CellRendererPixbuf());
//   cols_count = conflict_list->append_column(_("  "), *rendererpix);
//   column = conflict_list->get_column(cols_count-1);
//   column->add_attribute(rendererpix->property_pixbuf(), conflict_columns.selection);

//   renderer = Gtk::manage(new Gtk::CellRendererText());
//   cols_count = conflict_list->append_column(_("Remote value"), *renderer);
//   column = conflict_list->get_column(cols_count-1);
//   column->add_attribute(renderer->property_text(), conflict_columns.remote_value);
//   column->set_resizable(true);
//   renderer->property_editable().set_value(false);
//   renderer->property_style().set_value(Pango::STYLE_ITALIC);
  
//   Gtk::ScrolledWindow *conflict_scroll = manage(new Gtk::ScrolledWindow());
//   conflict_scroll->add(*conflict_list);

//   Gtk::VBox *conflictvbox = manage(new Gtk::VBox(true, 6));
//   conflictvbox->pack_start(*conflict_scroll, true, true, 0);

//   hbox->pack_start(*conflictvbox, true, true, 0);

//   gp->pack_start(*hbox, true, true, 0);

//   gp->show_all();
  
//   gp->set_border_width(12);

}


void
NetworkConflictDialog::create_model()
{
//   conflict_store = Gtk::ListStore::create(conflict_columns);

//   for (int i = 0; i < 10; i++)
//     {
//       Gtk::TreeRow row = *(conflict_store->append());

//       row[conflict_columns.name] = "name";
//       row[conflict_columns.local_value] = "local";
//       row[conflict_columns.remote_value] = "remote";

//       row[conflict_columns.selection] = pixbuf_left_arrow;
//     }
}


int
NetworkConflictDialog::run()
{
  TRACE_ENTER("NetworkConflictDialog::run");

  init();
  show_all();
  
  TRACE_EXIT();
  return 0;
}


void
NetworkConflictDialog::on_response(int response)
{
  (void) response;
  TRACE_ENTER("NetworkConflictDialog::on_response")
  TRACE_EXIT();
}

