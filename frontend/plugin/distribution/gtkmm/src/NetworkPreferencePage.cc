// NetworkPreferencePage.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2010 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#ifdef HAVE_DISTRIBUTION

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include <sstream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>

#include "NetworkPreferencePage.hh"
#include "Hig.hh"
#include "GtkUtil.hh"

#include "INetwork.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "DataConnector.hh"

using namespace std;
using namespace workrave;

NetworkPreferencePage::NetworkPreferencePage()
  : Gtk::VBox(false, 6)
{
  TRACE_ENTER("NetworkPreferencePage::NetworkPreferencePage");

  connector = new DataConnector();

  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos(Gtk::POS_TOP);

  create_general_page(tnotebook);
  create_peers_page(tnotebook);
  create_advanced_page(tnotebook);

  pack_start(*tnotebook, true, true, 0);

  tnotebook->show_all();
  tnotebook->set_current_page(0);

  TRACE_EXIT();
}


NetworkPreferencePage::~NetworkPreferencePage()
{
  TRACE_ENTER("NetworkPreferencePage::~NetworkPreferencePage");

  delete connector;
  TRACE_EXIT();
}


void
NetworkPreferencePage::create_general_page(Gtk::Notebook *tnotebook)
{
  // Main switch
  enabled_cb = Gtk::manage(new Gtk::CheckButton());
  Gtk::Label *ena_lab
    = Gtk::manage(GtkUtil::create_label(_("Enable networking"), true));
  enabled_cb->add(*ena_lab);

  // Identity
  HigCategoryPanel *id_frame = Gtk::manage(new HigCategoryPanel(*enabled_cb));
  username_entry = manage(new Gtk::Entry());
  secret1_entry = manage(new Gtk::Entry());
  secret2_entry = manage(new Gtk::Entry());

  id_frame->add(_("Username:"), *username_entry);
  id_frame->add(_("Shared secret:"), *secret1_entry);
  id_frame->add(_("Repeat:"), *secret2_entry);
  secret1_entry->set_visibility(false);
  secret1_entry->set_invisible_char('*');
  secret2_entry->set_visibility(false);
  secret2_entry->set_invisible_char('*');
 
  id_frame->set_border_width(12);
  tnotebook->append_page(*id_frame, _("General"));

  connector->connect("networking/enabled", dc::wrap(enabled_cb));
  connector->connect("networking/username", dc::wrap(username_entry));
  connector->connect("networking/secret", dc::wrap(secret1_entry, secret2_entry));
}


void
NetworkPreferencePage::create_advanced_page(Gtk::Notebook *tnotebook)
{
  HigCategoryPanel *advanced_frame
    = Gtk::manage(new HigCategoryPanel(_("Server settings")));
  advanced_frame->set_border_width(12);

  port_entry = manage(new Gtk::SpinButton());

  port_entry->set_range(1024, 65535);
  port_entry->set_increments(1, 10);
  port_entry->set_numeric(true);
  port_entry->set_width_chars(10);

  advanced_frame->add(_("Server port:"), *port_entry);

  advanced_frame->set_border_width(12);
  tnotebook->append_page(*advanced_frame, _("Advanced"));

  connector->connect("networking/port", dc::wrap(port_entry));
}


void
NetworkPreferencePage::create_peers_page(Gtk::Notebook *tnotebook)
{
  Gtk::VBox *gp = Gtk::manage(new Gtk::VBox(false, 6));
  gp->set_border_width(6);

  // Info text
  const char *label =
    _("The following list specifies the hosts that Workrave connects to on\n"
      "start-up. Click the host name or port number to edit.");

  Gtk::HBox *infohbox = Gtk::manage(new Gtk::HBox(false, 6));
  Gtk::Label *info_lab = Gtk::manage(new Gtk::Label(label));

  infohbox->pack_start(*info_lab, false, false, 0);
  gp->pack_start(*infohbox, false, false, 0);


  //
  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 6));

  peers_list = Gtk::manage(new Gtk::TreeView());
  peers_store = Gtk::ListStore::create(peers_columns);
  peers_list->set_model(peers_store);
  peers_list->set_rules_hint();

  // create tree view

  Glib::RefPtr<Gtk::TreeSelection> selection = peers_list->get_selection();
  selection->set_mode(Gtk::SELECTION_MULTIPLE);

  Gtk::CellRendererText *renderer = NULL;
  Gtk::TreeViewColumn *column = NULL;
  int cols_count = 0;

  renderer = Gtk::manage(new Gtk::CellRendererText());
  cols_count = peers_list->append_column(_("Host name"), *renderer);
  column = peers_list->get_column(cols_count-1);
  column->add_attribute(renderer->property_text(), peers_columns.hostname);
  column->set_resizable(true);
  renderer->property_editable().set_value(true);
  renderer->signal_edited().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_hostname_edited));
  peers_list->set_search_column(peers_columns.hostname.index());

  renderer = Gtk::manage(new Gtk::CellRendererText());
  cols_count = peers_list->append_column(_("Port"), *renderer);
  column = peers_list->get_column(cols_count-1);
  column->add_attribute(renderer->property_text(), peers_columns.port);
  column->set_resizable(true);
  renderer->property_editable().set_value(true);
  renderer->signal_edited().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_port_edited));

  
  Gtk::ScrolledWindow *peers_scroll = Gtk::manage(new Gtk::ScrolledWindow());
  peers_scroll->add(*peers_list);

  Gtk::VBox *peersvbox = Gtk::manage(new Gtk::VBox(true, 6));
  peersvbox->pack_start(*peers_scroll, true, true, 0);

  hbox->pack_start(*peersvbox, true, true, 0);

  peers_store->signal_row_changed().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_row_changed));
  peers_store->signal_row_inserted().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_row_changed));
  peers_store->signal_row_deleted().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_row_deleted));


  // Buttons
  remove_btn = Gtk::manage(new Gtk::Button(Gtk::Stock::REMOVE));
  remove_btn->signal_clicked().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_peer_remove));
  add_btn = Gtk::manage(new Gtk::Button(Gtk::Stock::ADD));
  add_btn->signal_clicked().connect(sigc::mem_fun(*this, &NetworkPreferencePage::on_peer_add));

  Gtk::VBox *btnbox= Gtk::manage(new Gtk::VBox(false, 6));
  btnbox->pack_start(*add_btn, false, false, 0);
  btnbox->pack_start(*remove_btn, false, false, 0);

  hbox->pack_start(*btnbox, false, false, 0);

  gp->pack_start(*hbox, true, true, 0);

  create_model();
  
  gp->show_all();

  gp->set_border_width(12);
  tnotebook->append_page(*gp, _("Hosts"));
}

void
NetworkPreferencePage::create_model()
{
  peers_store = Gtk::ListStore::create(peers_columns);

  string startup_peers;
  CoreFactory::get_configurator()->get_value("networking/peers", startup_peers);

// #ifdef PLATFORM_OS_WIN32
//   // FIXME: this seems to avoid a crash on windows...bug #791
//   Gtk::ListStore *store = peers_store.operator->();
//   printf("%x\n", store);
// #endif
  
  gchar **peer_list = g_strsplit(startup_peers.c_str(), ",", 0);

  for (int i = 0; peer_list[i] != NULL; i++)
    {
      string peer = peer_list[i];
      string hostname, port;

      if (peer != "")
        {
          Gtk::TreeIter iter = peers_store->append();
          Gtk::TreeRow row = *iter;

          string hostname, port;
          parse_peers(peer, hostname, port);
          
          row[peers_columns.hostname]  = hostname;
          row[peers_columns.port]      = port;
        }
    }

  g_strfreev(peer_list);
}


void
NetworkPreferencePage::parse_peers(const string &peer, string &hostname, string &port)
{
  hostname = "";
  port = "";
  
  std::string::size_type pos = peer.find("tcp://");
  if (pos != std::string::npos)
    {
      hostname = peer.substr(6);

      pos = hostname.rfind(":");
      if (pos != std::string::npos)
        {
          port = hostname.substr(pos + 1);
          hostname = hostname.substr(0, pos);
        }
    }
}


void
NetworkPreferencePage::on_peer_remove()
{
  TRACE_ENTER("NetworkPreferencePage::on_peer_remove");
  Glib::RefPtr<Gtk::TreeSelection> selection = peers_list->get_selection();

  const Gtk::TreeSelection::SlotForeachIter& slot =
    sigc::mem_fun(*this, &NetworkPreferencePage::remove_peer);

  selection->selected_foreach_iter(slot);

  Glib::RefPtr<Gtk::ListStore> new_store = Gtk::ListStore::create(peers_columns);

  typedef Gtk::TreeModel::Children type_children;
  type_children children = peers_store->children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring hostname = row[peers_columns.hostname];
      Glib::ustring port = row[peers_columns.port];

      if (hostname != "" || port != "")
        {
          Gtk::TreeRow new_row = *(new_store->append());

          new_row[peers_columns.hostname]  = hostname;
          new_row[peers_columns.port]      = port;
        }
    }

  peers_store = new_store;
  peers_list->set_model(peers_store);
  TRACE_EXIT();
}


void
NetworkPreferencePage::on_peer_add()
{
  TRACE_ENTER("NetworkPreferencePage::on_peer_add");

  stringstream ss;
  int port = (int) port_entry->get_value();
  ss << port;

  Gtk::TreeModel::iterator iter = peers_store->append();
  Gtk::TreeModel::Row row = *iter;
  
  row[peers_columns.hostname]  = "";
  row[peers_columns.port]      = ss.str();
  
  TRACE_EXIT();
}

void
NetworkPreferencePage::remove_peer(const Gtk::TreeModel::iterator &iter)
{
  Gtk::TreeModel::Row row = *iter;
  Glib::ustring s = row[peers_columns.hostname];
  row[peers_columns.hostname]  = "";
  row[peers_columns.port]      = "";
}

void
NetworkPreferencePage::on_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter)
{
  (void) path;
  (void) iter;
  update_peers();
}


void
NetworkPreferencePage::on_row_deleted(const Gtk::TreeModel::Path& path)
{
  (void) path;
  update_peers();
}


void
NetworkPreferencePage::on_hostname_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  GtkTreePath *gpath = gtk_tree_path_new_from_string (path_string.c_str());
  Gtk::TreePath path(gpath);

  /* get toggled iter */
  Gtk::TreeRow row = *(peers_store->get_iter(path));

  row[peers_columns.hostname]  = new_text;
}


void
NetworkPreferencePage::on_port_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  GtkTreePath *gpath = gtk_tree_path_new_from_string (path_string.c_str());
  Gtk::TreePath path(gpath);

  /* get toggled iter */
  Gtk::TreeRow row = *(peers_store->get_iter(path));

  row[peers_columns.port]  = new_text;
}


void
NetworkPreferencePage::update_peers()
{
  string peers;
  bool first = true;

  typedef Gtk::TreeModel::Children type_children;
  type_children children = peers_store->children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring hostname = row[peers_columns.hostname];
      Glib::ustring port = row[peers_columns.port];

      if (!first)
        {
          peers += ",";
        }

      if (hostname != "" && port != "")
        {
          peers += "tcp://" + hostname + ":" + port;
          first = false;
        }
    }

  CoreFactory::get_configurator()->set_value("networking/peers", peers);
}

#endif
