// TimerBox.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOX_HH
#define TIMERBOX_HH

#include "preinclude.h"

class GUI;
class TimeBar;
class NetworkLogDialog;

#include <gtkmm.h>

#include "GUIControl.hh"

using namespace std;

class TimerBox :
  public ConfiguratorListener,
  public Gtk::Table
{
public:  
  TimerBox(string name);
  ~TimerBox();

  void set_geometry(bool vertical, int size);
  void init();
  void update();

  static const string get_timer_config_key(string name, GUIControl::BreakId timer, const string &key);
  static int get_cycle_time(string name);
  static void set_cycle_time(string name, int time);
  static int get_timer_imminent_time(string name, GUIControl::BreakId timer);
  static void set_timer_imminent_time(string name, GUIControl::BreakId timer, int time);
  static int get_timer_slot(string name, GUIControl::BreakId timer);
  static void set_timer_slot(string name, GUIControl::BreakId timer, int slot);
  static int get_timer_flags(string name, GUIControl::BreakId timer);
  static void set_timer_flags(string name, GUIControl::BreakId timer, int flags);
  
  
public:
  static const string CFG_KEY_TIMERSBOX;
  static const string CFG_KEY_TIMERSBOX_HORIZONTAL;
  static const string CFG_KEY_TIMERSBOX_CYCLE_TIME;
  static const string CFG_KEY_TIMERSBOX_POSITION;
  static const string CFG_KEY_TIMERSBOX_FLAGS;
  static const string CFG_KEY_TIMERSBOX_IMMINENT;

  enum SlotType
    {
      BREAK_WHEN_IMMINENT = 1,
      BREAK_WHEN_FIRST = 2,
      BREAK_SKIP = 4,
      BREAK_EXCLUSIVE = 8,
      BREAK_DEFAULT = 16,
      BREAK_HIDE = 32
    };
  
  
private:
  // ConfiguratorListener
  void config_changed_notify(string key);
 
  void read_configuration();
 
  void init_widgets();
  void update_widgets();
  void init_table();
  void init_slot(int slot);
  void cycle_slots();
  
 
private:
  //! Parent container
  Gtk::Bin *parent;
  
  //! Array of time labels
  Gtk::Widget **labels;

  //! Array of time bar widgets.
  TimeBar **bars;

  //!
  Gtk::Image *sheep;
  
  //! Reconfigure the panel.
  bool reconfigure;

  //! Duration of each cycle.
  int cycle_time;

  //! Allign break vertically.
  bool vertical;

  //!
  int size;
  
  //! Positions for the break timers.
  int break_position[GUIControl::BREAK_ID_SIZEOF];

  //! Flags for the break timers.
  int break_flags[GUIControl::BREAK_ID_SIZEOF];

  //! Imminent threshold for the timers.
  int break_imminent_time[GUIControl::BREAK_ID_SIZEOF];
  
  //! Computed slot contents.
  int break_slots[GUIControl::BREAK_ID_SIZEOF][GUIControl::BREAK_ID_SIZEOF];

  //! Current cycle for each slot.
  int break_slot_cycle[GUIControl::BREAK_ID_SIZEOF];

  //! Current slot content.
  int current_content[GUIControl::BREAK_ID_SIZEOF];

  //! Number of visible breaks.
  int visible_count;

  //!
  string name;
};

#endif // TIMERBOX_HH
