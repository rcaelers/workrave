// NetworkPreferencesPage.hh --- Preferences for network
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2009 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef NETWORKPREFERENCEPAGE_HH
#define NETWORKPREFERENCEPAGE_HH

#include <stdio.h>
#include <string>

#include "preinclude.h"

#include <gtkmm/liststore.h>
#include <gtkmm/box.h>

class Configurator;
class DataConnector;

namespace Gtk
{
  class Label;
  class Entry;
  class CheckButton;
  class SpinButton;
  class Button;
  class Notebook;
}

namespace workrave
{
  class INetwork;
}

using namespace workrave;

class NetworkPreferencePage
  : public Gtk::VBox
{
public:
  NetworkPreferencePage();
  ~NetworkPreferencePage();

private:
  void create_general_page(Gtk::Notebook *tnotebook);
  void create_advanced_page(Gtk::Notebook *tnotebook);
  void create_peers_page(Gtk::Notebook *tnotebook);
  void create_model();

  void on_peer_remove();
  void on_peer_add();
  void on_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  void on_row_deleted(const Gtk::TreeModel::Path& path);
  void on_hostname_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_port_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);

  void update_peers();

  void remove_peer(const Gtk::TreeModel::iterator &iter);

  workrave::INetwork *network;

  Gtk::Entry *username_entry;
  Gtk::Label *secret1_label;
  Gtk::Entry *secret1_entry;
  Gtk::Label *secret2_label;
  Gtk::Entry *secret2_entry;
  Gtk::CheckButton *enabled_cb;
  Gtk::SpinButton *port_entry;

  Gtk::Button *remove_btn;
  Gtk::Button *add_btn;

  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<std::string> hostname;
    Gtk::TreeModelColumn<std::string> port;

    ModelColumns()
    {
      add(hostname);
      add(port);
    }
  };

  Gtk::TreeView *peers_list;
  Glib::RefPtr<Gtk::ListStore> peers_store;
  const ModelColumns peers_columns;

  DataConnector *connector;
};

#endif // NETWORKPREFERENCEPAGE_HH
