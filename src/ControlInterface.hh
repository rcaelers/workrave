// ControlInterface.hh --- The main controller interface
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
// $Id$
//

#ifndef CONTROLINTERFACE_HH
#define CONTROLINTERFACE_HH

#include <string>
#include <list>
#include <map>

#include "TimerInterface.hh"

class Timer;
class TimerInterface;
class ActivityMonitorInterface;
class Configurator;

class ControlInterface 
{
public:
  virtual ~ControlInterface() {}

  //! Initialize the Core Control. Must be called first.
  virtual void init(Configurator *config, char *display) = 0;

  //! Creates and adds the timer.
  virtual TimerInterface *create_timer(string id) = 0;

  //! Initializes all added timers.
  virtual void init_timers() = 0;

  //! Processes all timers.
  virtual void process_timers(map<string, TimerInfo> &infos) = 0;

  //! Returns the activity monitor.
  virtual ActivityMonitorInterface *get_activity_monitor() const = 0;

#ifndef NDEBUG
  virtual void test_me() = 0;
#endif
  
public:
  static const string CFG_KEY_TIMERS;
  static const string CFG_KEY_TIMER;
  static const string CFG_KEY_TIMER_LIMIT;
  static const string CFG_KEY_TIMER_AUTO_RESET;
  static const string CFG_KEY_TIMER_RESET_PRED;
  static const string CFG_KEY_TIMER_SNOOZE;
  static const string CFG_KEY_TIMER_RESTORE;
  static const string CFG_KEY_TIMER_COUNT_ACTIVITY;
  static const string CFG_KEY_TIMER_MONITOR;
  static const string CFG_KEY_MONITOR;
  static const string CFG_KEY_MONITOR_NOISE;
  static const string CFG_KEY_MONITOR_ACTIVITY;
  static const string CFG_KEY_MONITOR_IDLE;
};

#endif // CONTROLINTERFACE_HH
