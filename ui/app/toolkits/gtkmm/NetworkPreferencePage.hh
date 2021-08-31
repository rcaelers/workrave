// NetworkPreferencesPage.hh --- Preferences for network
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2010, 2013 Rob Caelers & Raymond Penners
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

#ifndef NETWORKPREFERENCEPAGE_HH
#define NETWORKPREFERENCEPAGE_HH

#include <cstdio>
#include <string>

#include "IApplication.hh"

#include <gtkmm/liststore.h>
#include <gtkmm/box.h>

class Configurator;

namespace workrave
{
  class IDistributionManager;
}

namespace Gtk
{
  class Label;
  class Entry;
  class CheckButton;
  class SpinButton;
  class Button;
  class Notebook;
} // namespace Gtk

#include <gtkmm/box.h>

class NetworkPreferencePage : public Gtk::VBox
{
public:
  NetworkPreferencePage(std::shared_ptr<IApplication> app);
  ~NetworkPreferencePage() override;

private:
  void init_page_values();
  void create_general_page(Gtk::Notebook *tnotebook);
  void create_advanced_page(Gtk::Notebook *tnotebook);
  void create_peers_page(Gtk::Notebook *tnotebook);
  void create_model();

  void on_enabled_toggled();
  void on_listening_toggled();
  void on_username_changed();
  void on_password_changed();
  void on_port_changed();
  void on_interval_changed();
  void on_attempts_changed();

  void on_peer_remove();
  void on_peer_add();
  void on_row_changed(const Gtk::TreeModel::Path &path, const Gtk::TreeModel::iterator &iter);
  void on_row_deleted(const Gtk::TreeModel::Path &path);
  void on_hostname_edited(const Glib::ustring &path_string, const Glib::ustring &new_text);
  void on_port_edited(const Glib::ustring &path_string, const Glib::ustring &new_text);

  void update_peers();

  void remove_peer(const Gtk::TreeModel::iterator &iter);
  void parse_peers(const std::string &peer, std::string &hostname, std::string &port);

  std::shared_ptr<IApplication> app;
  workrave::IDistributionManager *dist_manager{nullptr};

  Gtk::Entry *username_entry{nullptr};
  Gtk::Entry *password_entry{nullptr};
  Gtk::CheckButton *enabled_cb{nullptr};
  Gtk::CheckButton *listening_cb{nullptr};
  Gtk::SpinButton *port_entry{nullptr};
  Gtk::SpinButton *attempts_entry{nullptr};
  Gtk::SpinButton *interval_entry{nullptr};

  Gtk::Button *remove_btn{nullptr};
  Gtk::Button *add_btn{nullptr};

  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<Glib::ustring> hostname;
    Gtk::TreeModelColumn<Glib::ustring> port;

    ModelColumns()
    {
      add(hostname);
      add(port);
    }
  };

  Gtk::TreeView *peers_list{nullptr};
  Glib::RefPtr<Gtk::ListStore> peers_store;
  ModelColumns peers_columns;
};

#endif // NETWORKPREFERENCEPAGE_HH
