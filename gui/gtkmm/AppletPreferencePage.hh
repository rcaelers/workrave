// AppletPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef APPLETPREFERENCEPAGE_HH
#define APPLETPREFERENCEPAGE_HH

#include <stdio.h>
#include <string>

#include "preinclude.h"

class Configurator;

#include <gtkmm.h>

class AppletPreferencePage
  : public Gtk::HBox
{
public:  
  AppletPreferencePage();
  ~AppletPreferencePage();
  
private:
  void init_page_values();
  void create_page();

  void set_flag(int break_id, int flag, bool on);
  void on_first_toggled(int break_id);
  void on_imminent_toggled(int break_id);
  void on_default_toggled(int break_id);
  void on_exclusive_toggled(int break_id);
  void on_time_changed(int break_id);
  void on_slot_changed(int break_id);
  void on_visible_toggled(int break_id);

  void on_enabled_toggled();
  void on_cycle_time_changed();
  
  Gtk::SpinButton *cycle_entry;
  Gtk::CheckButton *enabled_cb;
  Gtk::CheckButton *visible_cb[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::SpinButton *slot_entry[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::CheckButton *first_cb[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::CheckButton *imminent_cb[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::CheckButton *exclusive_cb[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::CheckButton *default_cb[GUIControl::BREAK_ID_SIZEOF];  
  Gtk::SpinButton *time_entry[GUIControl::BREAK_ID_SIZEOF];  
};

#endif // APPLETPREFERENCEPAGE_HH
