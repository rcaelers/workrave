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

#include <string>
#include <gtkmm/table.h>

#include "CoreInterface.hh"
#include "ConfiguratorListener.hh"

class TimeBar;
namespace Gtk
{
  class Image;
  class Bin;
}

using namespace std;

class TimerBox :
  public ConfiguratorListener,
  public Gtk::Table
{
public:  
  TimerBox(std::string name);
  ~TimerBox();

  void set_geometry(bool vertical, int size);
  void init();
  void update();

  int get_visible_count() const;
  
  static const string get_timer_config_key(string name, BreakId timer, const string &key);
  static int get_cycle_time(string name);
  static void set_cycle_time(string name, int time);
  static int get_timer_imminent_time(string name, BreakId timer);
  static void set_timer_imminent_time(string name, BreakId timer, int time);
  static int get_timer_slot(string name, BreakId timer);
  static void set_timer_slot(string name, BreakId timer, int slot);
  static int get_timer_flags(string name, BreakId timer);
  static void set_timer_flags(string name, BreakId timer, int flags);
  static bool is_enabled(string name);
  static void set_enabled(string name, bool enabled);
  
public:
  static const string CFG_KEY_TIMERBOX;
  static const string CFG_KEY_TIMERBOX_HORIZONTAL;
  static const string CFG_KEY_TIMERBOX_CYCLE_TIME;
  static const string CFG_KEY_TIMERBOX_POSITION;
  static const string CFG_KEY_TIMERBOX_FLAGS;
  static const string CFG_KEY_TIMERBOX_IMMINENT;
  static const string CFG_KEY_TIMERBOX_ENABLED;

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

  //!
  int table_rows;

  //!
  int table_columns;
  
  //! Positions for the break timers.
  int break_position[BREAK_ID_SIZEOF];

  //! Flags for the break timers.
  int break_flags[BREAK_ID_SIZEOF];

  //! Imminent threshold for the timers.
  int break_imminent_time[BREAK_ID_SIZEOF];
  
  //! Computed slot contents.
  int break_slots[BREAK_ID_SIZEOF][BREAK_ID_SIZEOF];

  //! Current cycle for each slot.
  int break_slot_cycle[BREAK_ID_SIZEOF];

  //! Current slot content.
  int current_content[BREAK_ID_SIZEOF];

  //! Number of visible breaks.
  int visible_count;

  //!
  string name;
};


inline int
TimerBox::get_visible_count() const
{
  return visible_count;
}

#endif // TIMERBOX_HH
