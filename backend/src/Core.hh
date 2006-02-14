// Core.hh --- The main controller
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#ifndef CORE_HH
#define CORE_HH

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

#include "Break.hh"
#include "BreakResponseInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "CoreInterface.hh"
#include "CoreEventListener.hh"
#include "ConfiguratorListener.hh"
#include "TimeSource.hh"
#include "Timer.hh"
#include "Statistics.hh"

class ActivityMonitor;
class Configurator;
class Statistics;
class SoundPlayerInterface;
class AppInterface;
class FakeActivityMonitor;
class IdleLogManager;
class BreakControl;

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "DistributionClientMessageInterface.hh"
#include "DistributionListener.hh"
#endif

class Core :
#ifdef HAVE_DISTRIBUTION
  public DistributionClientMessageInterface,
  public DistributionListener,
#endif  
  public TimeSource,
  public CoreInterface,
  public ConfiguratorListener,
  public BreakResponseInterface
{
public:
  Core();
  virtual ~Core();
    
  static const string CFG_KEY_MONITOR;
  static const string CFG_KEY_MONITOR_NOISE;
  static const string CFG_KEY_MONITOR_ACTIVITY;
  static const string CFG_KEY_MONITOR_IDLE;
  static const string CFG_KEY_GENERAL_DATADIR;
  
  static Core *get_instance();

  Timer *get_timer(string name) const;
  Timer *get_timer(BreakId id) const;
  Break *get_break(BreakId id);
  Configurator *get_configurator() const;
  ActivityMonitorInterface *get_activity_monitor() const;
#ifdef HAVE_DISTRIBUTION
  DistributionManager *get_distribution_manager() const;
#endif
  Statistics *get_statistics() const;
  void set_core_events_listener(CoreEventListener *l);
  void force_break(BreakId id, bool initiated_by_user);
  void set_powersave(bool down);
 
  time_t get_time() const;
  void post_event(CoreEvent event);

  OperationMode get_operation_mode();
  OperationMode set_operation_mode(OperationMode mode);
  void set_freeze_all_breaks(bool freeze);

  void stop_prelude(BreakId break_id);
  void do_force_break(BreakId id, bool initiated_by_user);

  void freeze();
  void defrost();

  void force_idle();
  
  ActivityState get_current_monitor_state() const;
  bool is_master() const;
  
private:

#ifndef NDEBUG
  enum ScriptCommand
    {
      SCRIPT_START = 1,
    };
#endif  
  
  void init(int argc, char **argv, AppInterface *application, char *display_name);
  void init_breaks();
  void init_configurator();
  void init_monitor(char *display_name);
  void init_distribution_manager();
  void init_statistics();
  void load_monitor_config();
  void config_changed_notify(string key);
  void heartbeat();
  void timer_action(BreakId id, TimerInfo info);
  void process_distribution();
  void process_state();
  void process_timewarp();
  void process_timers();
  void start_break(BreakId break_id, BreakId resume_this_break = BREAK_ID_NONE);
  void stop_all_breaks();
  void daily_reset();
  void save_state() const;
  void load_state();
  void do_postpone_break(BreakId break_id);
  void do_skip_break(BreakId break_id);
  void do_stop_prelude(BreakId break_id);
#ifndef NDEBUG
  void test_me();
#endif

  void set_insist_policy(CoreInterface::InsistPolicy p);
  CoreInterface::InsistPolicy get_insist_policy() const;

#ifdef HAVE_DISTRIBUTION
  bool request_client_message(DistributionClientMessageID id, PacketBuffer &buffer);
  bool client_message(DistributionClientMessageID id, bool master, const char *client_id,
                      PacketBuffer &buffer);

  bool request_break_state(PacketBuffer &buffer);
  bool set_break_state(bool master, PacketBuffer &buffer);
  
  bool request_timer_state(PacketBuffer &buffer) const;
  bool set_timer_state(PacketBuffer &buffer);

  bool set_monitor_state(bool master, PacketBuffer &buffer);

  enum BreakControlMessage
    {
      BCM_POSTPONE,
      BCM_SKIP,
      BCM_ABORT_PRELUDE,
      BCM_START_BREAK,
    };
  
  void send_break_control_message(BreakId break_id, BreakControlMessage message);
  void send_break_control_message_bool_param(BreakId break_id, BreakControlMessage message,
                                             bool param);
  bool set_break_control(PacketBuffer &buffer);
  
  void signon_remote_client(string client_id);
  void signoff_remote_client(string client_id);
  void compute_timers();
#ifndef NDEBUG
  bool script_message(bool master, const char *client_id, PacketBuffer &buffer);
  void do_script();
#endif // NDEBUG
#endif // HAVE_DISTRIBUTION

  // BreakResponseInterface
  void postpone_break(BreakId break_id);
  void skip_break(BreakId break_id);
  
private:
  //! The one and only instance
  static Core *instance;

  //! Number of command line arguments passed to the program.
  int argc;

  //! Command line arguments passed to the program.
  char **argv;
  
  //! The current time.
  time_t current_time;

  //! The time we last processed the timers.
  time_t last_process_time;
  
  //! Are we the master node??
  bool master_node;

  //! List of breaks.
  Break breaks[BREAK_ID_SIZEOF];

  //! The Configurator.
  Configurator *configurator;

  //! The activity monitor
  ActivityMonitor *monitor;

  //! GUI Widget factory.
  AppInterface *application;
  
  //! The statistics collector.
  Statistics *statistics;
  
  //! Current operation mode.
  OperationMode operation_mode;

  //! Where to send core events to?
  CoreEventListener *core_event_listener;

  //! Did the OS announce a powersave?
  bool powersave;

  //! Time the OS announces a resume from powersave
  time_t powersave_resume_time;

  //! OperationMode before powersave
  OperationMode powersave_operation_mode;

  //! What to do with activity during insisted break?
  CoreInterface::InsistPolicy insist_policy;

  //! Policy currently in effect.
  CoreInterface::InsistPolicy active_insist_policy;

  //! Resumes this break if current break ends.
  BreakId resume_break;
  
  //! Current local monitor state.
  ActivityState local_state;

  //! Current overall monitor state.
  ActivityState monitor_state;

#ifdef HAVE_DISTRIBUTION
  //! The Distribution Manager
  DistributionManager *dist_manager;

  //! State of the remote master.
  ActivityState remote_state;
  
  //! Manager that collects idle times of all clients.
  IdleLogManager *idlelog_manager;

#ifndef NDEBUG
  //! A fake activity monitor for testing puposes.
  FakeActivityMonitor *fake_monitor;

  //! Program Counter of script execution.
  int script_count;

  //! Time at which script execution started.
  int script_start_time;
#endif
#endif
};


//! Returns the singleton Core instance.
inline Core *
Core::get_instance()
{
  if (instance == NULL)
    {
      instance = new Core();
    }
       
  return instance;
}

//!
inline ActivityState
Core::get_current_monitor_state() const
{
  return monitor_state;
}

//!
inline bool
Core::is_master() const
{
  return master_node;
}

#endif // CORE_HH
