// Break.hh
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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
// $Id$%
//

#ifndef BREAK_HH
#define BREAK_HH

#include "CoreInterface.hh"
#include "ConfiguratorListener.hh"
#include "BreakInterface.hh"
#include "Timer.hh"

class Configurator;
class AppInterface;
class BreakControl;

using namespace std;

class Break :
  public BreakInterface,
  public ConfiguratorListener
{
private:
  //! ID of the break.
  BreakId break_id;

  //! Name of the break (used in configuration)
  string break_name;

  //! Timer config prefix
  string timer_prefix;

  //! Break config prefix
  string break_prefix;
  
  //! The Configurator
  Configurator *configurator;

  //!
  AppInterface *application;
  
  //! Interface pointer to the timer.
  Timer *timer;

  //! Interface pointer to the break controller.
  BreakControl *break_control;

  //! Break enabled?
  bool enabled;

public:
  Break();
  virtual ~Break();

  void init(BreakId id, AppInterface *app);

  bool is_enabled() const;
  string get_name() const;
  Timer *get_timer() const;
  BreakControl *get_break_control();
  
  int get_timer_limit() const;
  void set_timer_limit(int n);
  int get_timer_auto_reset() const;
  void set_timer_auto_reset(int n);
  string get_timer_reset_pred() const;
  void set_timer_reset_pred(string n);
  int get_timer_snooze() const;
  void set_timer_snooze(int n);
  string get_timer_monitor() const;
  void set_timer_monitor(string n);
  int get_break_max_preludes() const;
  void set_break_max_preludes(int n);
  bool get_break_force_after_preludes() const;
  void set_break_force_after_preludes(bool b);
  bool get_break_ignorable() const;
  void set_break_ignorable(bool b);
  int get_break_exercises() const;
  void set_break_exercises(int n);
  bool get_break_insisting() const;
  void set_break_insisting(bool b);
  bool get_break_enabled() const;
  void set_break_enabled(bool b);
  void config_changed_notify(string key);
  void set_insist_policy(InsistPolicy p);
  InsistPolicy get_insist_policy() const;

private:
  void init_timer();
  void update_timer_config();
  void load_timer_config();
  void load_timer_monitor_config();
  void init_break_control();
  void update_break_config();
  void load_break_control_config();

  bool startsWith(string &key, string prefix, string &timer_name);
  
private:
  static const string CFG_KEY_TIMER_PREFIX;
  
  static const string CFG_KEY_TIMER_LIMIT;
  static const string CFG_KEY_TIMER_AUTO_RESET;
  static const string CFG_KEY_TIMER_RESET_PRED;
  static const string CFG_KEY_TIMER_SNOOZE;
  static const string CFG_KEY_TIMER_MONITOR;

  static const string CFG_KEY_BREAK_PREFIX;
  
  static const string CFG_KEY_BREAK_MAX_PRELUDES;
  static const string CFG_KEY_BREAK_FORCE_AFTER_PRELUDES;
  static const string CFG_KEY_BREAK_IGNORABLE;
  static const string CFG_KEY_BREAK_INSISTING;
  static const string CFG_KEY_BREAK_ENABLED;
  static const string CFG_KEY_BREAK_EXERCISES;
};


#endif // TIMERDATA_HH
