// Control.hh --- The main controller
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

#ifndef CONTROL_HH
#define CONTROL_HH

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <iostream>
#include <string>

#include "ControlInterface.hh"
#include "ActivityMonitor.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributedStateInterface.hh"
#endif

#include "Timer.hh"
#include "TimeSource.hh"
#include "Configurator.hh"
#include "ConfiguratorListener.hh"


#ifndef NDEBUG
class FakeActivityMonitor;
#endif

class DistributionManager;

class Control :
  public TimeSource,
  public ControlInterface,
  public ConfiguratorListener
#ifdef HAVE_DISTRIBUTION
  , public DistributedStateInterface
#endif  
{
public:
  typedef std::list<Timer *> Timers;
  typedef Timers::iterator TimerIter;
  typedef Timers::const_iterator TimerCIter;

  //! Contructor
  Control();

  //! Destructor
  virtual ~Control();

  // TimeSource methods
  time_t get_time() const;

  // ControlInterface methods.
  void init(Configurator *config, char *display_name);
  void terminate();
  void process_timers(map<string, TimerInfo> &infos);
  void init_timers();
  TimerInterface *create_timer(string id);

  ActivityMonitor *get_activity_monitor() const
  {
    return monitor;
  }

  virtual void config_changed_notify(string key);

private:
  void save_state() const;
  void load_state();

  Timer *get_timer(string id);
  void configure_timer(string id, Timer *timer);
  void configure_timer_monitor(string id, Timer *timer);
  void load_monitor_config();
  void store_monitor_config();

  bool create_monitor(char *display_name);
  bool create_timers();

#ifdef HAVE_DISTRIBUTION
  bool create_distribution_manager();
  bool get_state(DistributedStateID id, unsigned char **buffer, int *size);
  bool set_state(DistributedStateID id, bool master, unsigned char *buffer, int size);
#endif
  
private:
  //! List of timers
  Timers timers;

  //! The Configurator.
  Configurator *configurator;

  //! The activity monitor
  ActivityMonitor *monitor;

  //! The current time.
  time_t current_time;

#ifdef HAVE_DISTRIBUTION
  //! The Distribution Manager
  DistributionManager *dist_manager;
#endif

  //! Are we the master node??
  bool master_node;
  
#ifndef NDEBUG
  FakeActivityMonitor *fake_monitor;
#endif  
};

#endif // CONTROL_HH
