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

#ifndef TIMERBOXPREFERENCEPAGE_HH
#define TIMERBOXPREFERENCEPAGE_HH

class Configurator;

#include "GUIControl.hh"

#include <gtkmm.h>

class TimerBoxPreferencePage
  : public Gtk::HBox
{
public:  
  TimerBoxPreferencePage(string name);
  ~TimerBoxPreferencePage();
  
private:
  void init_page_values();
  void create_page();
  void enable_buttons();
  void on_enabled_toggled();
  void on_place_changed();
  void on_display_changed(int break_id);
  void on_cycle_time_changed();
  void on_always_on_top_toggled();

  string name;
  
  Gtk::CheckButton *ontop_cb;
  Gtk::CheckButton *enabled_cb;
  Gtk::OptionMenu *place_button;
  Gtk::OptionMenu *timer_display_button[GUIControl::BREAK_ID_SIZEOF];
  Gtk::SpinButton *cycle_entry;
};

#endif // TIMERBOXPREFERENCEPAGE_HH
