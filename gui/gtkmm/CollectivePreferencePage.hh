// TimerPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#ifndef COLLECTIVEPREFERENCEPAGE_HH
#define COLLECTIVEPREFERENCEPAGE_HH

#include <stdio.h>
#include <string>

#include "preinclude.h"

class Configurator;

#include <gtkmm.h>

class CollectivePreferencePage
  : public Gtk::HBox
{
public:  
  CollectivePreferencePage();
  ~CollectivePreferencePage();
  
private:
  void init_page_values();
  void create_general_page(Gtk::Notebook *tnotebook);
  void create_advanced_page(Gtk::Notebook *tnotebook);
  void create_peers_page(Gtk::Notebook *tnotebook);
  void create_model();

  void on_enabled_toggled();
  void on_username_changed();
  void on_password_changed();
  void on_port_changed();
  void on_interval_changed();
  void on_attempts_changed();

  void on_peer_remove();
  void on_peer_add();
  void on_row_changed(const Gtk::TreeModel::Path& path, const Gtk::TreeModel::iterator& iter);
  void on_row_deleted(const Gtk::TreeModel::Path& path);
  void on_hostname_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
  void on_port_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
  
  void update_peers();
  void remove_peer(const Gtk::TreeModel::iterator &iter);
  
  Gtk::Label *password2_label;
  Gtk::Entry *peers_entry;
  Gtk::Entry *username_entry;
  Gtk::Entry *password_entry;
  Gtk::CheckButton *enabled_cb;
  Gtk::SpinButton *port_entry;
  Gtk::SpinButton *attempts_entry;
  Gtk::SpinButton *interval_entry;

  Gtk::Button *remove_btn;
  Gtk::Button *add_btn;
  
  struct ModelColumns : public Gtk::TreeModelColumnRecord
  {
    Gtk::TreeModelColumn<string> hostname;
    Gtk::TreeModelColumn<string> port;

    ModelColumns()
    {
      add(hostname);
      add(port);
    }
  };

  Gtk::TreeView *peers_list;
  Glib::RefPtr<Gtk::ListStore> peers_store;
  const ModelColumns peers_columns;
};

#endif // COLLECTIVEPREFERENCEPAGE_HH
