// TimerPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef TIMERPREFERENCESPANEL_HH
#define TIMERPREFERENCESPANEL_HH

#include <stdio.h>
#include <string>

#include "preinclude.h"

class TimerInterface;
class Configurator;

#include <gtkmm.h>
#include "ConfiguratorListener.hh"
#include "GUIControl.hh"

class TimeEntry;

class TimerPreferencesPanel
  : public Gtk::HBox,
    public ConfiguratorListener
{
public:  
  TimerPreferencesPanel(GUIControl::TimerId timer);
  ~TimerPreferencesPanel();

  virtual void config_changed_notify(string key);
  void on_snooze_changed();
  void on_auto_reset_changed();
  void on_limit_changed();
  void on_ignorable_toggled();
  void on_insists_toggled();
  void on_monitor_toggled();
  void on_preludes_active_toggled();
  void on_preludes_maximum_toggled();
  void on_preludes_force_toggled();
  
private:
  Gtk::Frame *create_prelude_frame();
  Gtk::Frame *create_options_frame();
  Gtk::Frame *create_timers_frame();
  
  GUIControl::TimerId timer_id;
  GUIControl::TimerData *timer;

  Gtk::CheckButton *insists_cb;
  Gtk::CheckButton *ignorable_cb;
  Gtk::CheckButton *monitor_cb;
  Gtk::CheckButton *prelude_cb;
  Gtk::CheckButton *has_max_prelude_cb;
  Gtk::CheckButton *force_after_prelude_cb;
  TimeEntry *limit_tim, *auto_reset_tim, *snooze_tim;
  Gtk::SpinButton *max_prelude_spin;
  Gtk::Adjustment max_prelude_adjustment;
};

#endif // TIMERPREFERENCESPANEL_HH
