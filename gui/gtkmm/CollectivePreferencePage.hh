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
  : public Gtk::VBox
{
public:  
  CollectivePreferencePage();
  ~CollectivePreferencePage();
  
private:
  void init_page_values();
  void create_general_page(Gtk::Notebook *tnotebook);
  void create_advanced_page(Gtk::Notebook *tnotebook);
  void create_peers_page(Gtk::Notebook *tnotebook);

  void on_enabled_toggled();
  void on_username_changed();
  void on_password_changed();
  void on_port_changed();
  void on_interval_changed();
  void on_attempts_changed();
  void on_peers_changed();

  Gtk::Label *password2_label;
  Gtk::Entry *peers_entry;
  Gtk::Entry *username_entry;
  Gtk::Entry *password1_entry;
  Gtk::Entry *password2_entry;
  Gtk::CheckButton *enabled_cb;
  Gtk::SpinButton *port_entry;
  Gtk::SpinButton *attempts_entry;
  Gtk::SpinButton *interval_entry;
};

#endif // COLLECTIVEPREFERENCEPAGE_HH
