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
#include "DistributionClientMessageInterface.hh"
#include "DistributionListener.hh"
#endif

#include "Timer.hh"
#include "TimeSource.hh"
#include "Configurator.hh"
#include "ConfiguratorListener.hh"


#ifndef NDEBUG
class FakeActivityMonitor;
#endif

class DistributionManager;
class IdleLogManager;
class PacketBuffer;

class Control :
  public TimeSource,
  public ControlInterface,
  public ConfiguratorListener
#ifdef HAVE_DISTRIBUTION
  ,
  public DistributionClientMessageInterface,
  public DistributionListener
#endif  
{
public:
  //! Contructor
  Control();

  //! Destructor
  virtual ~Control();

  // TimeSource methods
  time_t get_time() const;

  // ControlInterface methods.
  void init(int timer_count, Configurator *config, char *display_name);
  void terminate();
  void process_timers(TimerInfo *infos);
  void init_timers();
  void daily_reset();
  
  TimerInterface *create_timer(int timer_id, string name);
  ActivityMonitorInterface *get_activity_monitor() const
  {
    return monitor;
  }

  virtual void config_changed_notify(string key);

#ifndef NDEBUG
  void test_me();
#endif  

private:
#ifndef NDEBUG
  enum ScriptCommand
    {
      SCRIPT_START = 1,
    };
#endif  

  void save_state() const;
  void load_state();

  Timer *get_timer(string id);
  void configure_timer(Timer *timer);
  void configure_timer_monitor(Timer *timer);
  void load_monitor_config();
  void store_monitor_config();

  bool create_monitor(char *display_name);

#ifdef HAVE_DISTRIBUTION
  bool create_distribution_manager();
  bool request_client_message(DistributionClientMessageID id, PacketBuffer &buffer);
  bool client_message(DistributionClientMessageID id, bool master, const char *client_id,
                      PacketBuffer &buffer);
  
  bool get_timer_state(PacketBuffer &buffer) const;
  bool set_timer_state(PacketBuffer &buffer);
  bool set_monitor_state(bool master, PacketBuffer &buffer);

  void signon_remote_client(string client_id);
  void signoff_remote_client(string client_id);

  void compute_timers();
  
#ifndef NDEBUG
  bool script_message(bool master, const char *client_id, PacketBuffer &buffer);
  void do_script();
#endif  
#endif

private:
#ifdef HAVE_DISTRIBUTION
  //! The Distribution Manager
  DistributionManager *dist_manager;

  //! Current monitor state.
  ActivityState monitor_state;

  //! State of the remote master.
  ActivityState remote_state;
  
  //! 
  IdleLogManager *idlelog_manager;

#ifndef NDEBUG
  int script_count;

  int script_start_time;
#endif
#endif

#ifndef NDEBUG
  FakeActivityMonitor *fake_monitor;
#endif  

  //! Number of timers.
  int timer_count;

  //! List of timers
  Timer **timers;

  //! The Configurator.
  Configurator *configurator;

  //! The activity monitor
  ActivityMonitor *monitor;

  //! The current time.
  time_t current_time;

  //! The time we last processed the timers.
  time_t last_process_time;
  
  //! Are we the master node??
  bool master_node;
};

#endif // CONTROL_HH
