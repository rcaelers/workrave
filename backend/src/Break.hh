// Break.hh
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

#include "ICore.hh"
#include "IConfiguratorListener.hh"
#include "IBreak.hh"
#include "Timer.hh"

class Configurator;
class IApp;
class BreakControl;

using namespace std;

class Break :
  public IBreak,
  public IConfiguratorListener
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
  IApp *application;

  //! Interface pointer to the timer.
  Timer *timer;

  //! Interface pointer to the break controller.
  BreakControl *break_control;

  //! Break enabled?
  bool enabled;

public:
  Break();
  virtual ~Break();

  void init(BreakId id, IApp *app);

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
  bool get_timer_activity_sensitive() const;
  void set_timer_activity_sensitive(bool b);
  int get_break_max_preludes() const;
  void set_break_max_preludes(int n);
  int get_break_max_postpone() const;
  void set_break_max_postpone(int n);
  bool get_break_ignorable() const;
  void set_break_ignorable(bool b);
  int get_break_exercises() const;
  void set_break_exercises(int n);
  bool get_break_enabled() const;
  void set_break_enabled(bool b);
  void config_changed_notify(const string &key);

private:
  void init_timer();
  void update_timer_config();
  void load_timer_config();
  void load_timer_monitor_config();
  void init_break_control();
  void update_break_config();
  void load_break_control_config();

  bool starts_with(const string &key, string prefix, string &timer_name);

public:
  static const string CFG_KEY_TIMER_PREFIX;

  static const string CFG_KEY_TIMER_LIMIT;
  static const string CFG_KEY_TIMER_AUTO_RESET;
  static const string CFG_KEY_TIMER_RESET_PRED;
  static const string CFG_KEY_TIMER_SNOOZE;
  static const string CFG_KEY_TIMER_MONITOR;
  static const string CFG_KEY_TIMER_ACTIVITY_SENSITIVE;

  static const string CFG_KEY_BREAK_PREFIX;

  static const string CFG_KEY_BREAK_MAX_PRELUDES;
  static const string CFG_KEY_BREAK_MAX_POSTPONE;
  static const string CFG_KEY_BREAK_FORCE_AFTER_PRELUDES;
  static const string CFG_KEY_BREAK_IGNORABLE;
  static const string CFG_KEY_BREAK_ENABLED;
  static const string CFG_KEY_BREAK_EXERCISES;
};


#endif // TIMERDATA_HH
