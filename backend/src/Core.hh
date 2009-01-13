// Core.hh --- The main controller
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <map>

#include "Break.hh"
#include "IBreakResponse.hh"
#include "IActivityMonitor.hh"
#include "ICore.hh"
#include "ICoreEventListener.hh"
#include "IConfiguratorListener.hh"
#include "TimeSource.hh"
#include "Timer.hh"
#include "Statistics.hh"

#ifdef HAVE_DISTRIBUTION
#include "ILinkEventListener.hh"
#endif

using namespace workrave;

// Forward declarion of external interface.
namespace workrave {
  class ISoundPlayer;
  class IApp;
  class INetwork;
  class DBus;
}

class ActivityMonitor;
class Configurator;
class Statistics;
class FakeActivityMonitor;
class IdleLogManager;
class BreakControl;
class Network;
class BreakLinkEvent;
class CoreLinkEvent;
class TimerStateLinkEvent;

class Core :
  public TimeSource,
  public ICore,
  public IConfiguratorListener,
#ifdef HAVE_DISTRIBUTION  
  public ILinkEventListener,
#endif
  public IBreakResponse
{
public:
  Core();
  virtual ~Core();

  static Core *get_instance();

  Timer *get_timer(std::string name) const;
  Timer *get_timer(BreakId id) const;
  Break *get_break(BreakId id);
  Break *get_break(std::string name);
  Configurator *get_configurator() const;
  IActivityMonitor *get_activity_monitor() const;
  bool is_user_active() const;

  Statistics *get_statistics() const;
  void set_core_events_listener(ICoreEventListener *l);
  void force_break(BreakId id, bool initiated_by_user);
  void time_changed();
  void set_powersave(bool down);

  time_t get_time() const;
  void post_event(CoreEvent event);

  OperationMode get_operation_mode();
  OperationMode set_operation_mode(OperationMode mode, bool persistent = true);
  OperationMode set_operation_mode_no_event(OperationMode mode, bool persistent = true);
  void set_freeze_all_breaks(bool freeze);

  void stop_prelude(BreakId break_id);
  void do_force_break(BreakId id, bool initiated_by_user);

  void freeze();
  void defrost();

  void force_idle();

  ActivityState get_current_monitor_state() const;

  // DBus functions.
  void report_external_activity(std::string who, bool act);
  void is_timer_running(BreakId id, bool &value);
  void get_timer_elapsed(BreakId id,int *value);
  void get_timer_idle(BreakId id, int *value);
  void get_timer_overdue(BreakId id,int *value);
  
#ifdef HAVE_DISTRIBUTION
  Network *get_networking()
  {
    return (Network *)network;
  }

  void event_received(LinkEvent *event);  
  void break_event_received(const BreakLinkEvent *event);
  void core_event_received(const CoreLinkEvent *event);
  void timer_state_event_received(const TimerStateLinkEvent *event);
  void broadcast_state();
#endif
  
  // BreakResponseInterface
  void postpone_break(BreakId break_id);
  void skip_break(BreakId break_id);
  
#ifdef HAVE_DBUS
  DBus *get_dbus()
  {
    return dbus;
  }
#endif
  
private:

#ifndef NDEBUG
  enum ScriptCommand
    {
      SCRIPT_START = 1,
    };
#endif

  void init(int argc, char **argv, IApp *application, const std::string &display_name);
  void init_breaks();
  void init_configurator();
  void init_monitor(const std::string &display_name);
  void init_networking();
  void init_bus();
  void init_statistics();

  void load_monitor_config();
  void config_changed_notify(const std::string &key);
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
  void load_misc();
  void do_postpone_break(BreakId break_id);
  void do_skip_break(BreakId break_id);
  void do_stop_prelude(BreakId break_id);

  void set_insist_policy(ICore::InsistPolicy p);
  ICore::InsistPolicy get_insist_policy() const;


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

  //! List of breaks.
  Break breaks[BREAK_ID_SIZEOF];

  //! The Configurator.
  Configurator *configurator;

  //! The activity monitor
  ActivityMonitor *monitor;

  //! GUI Widget factory.
  IApp *application;

  //! The statistics collector.
  Statistics *statistics;

  //! Current operation mode.
  OperationMode operation_mode;

  //! Where to send core events to?
  ICoreEventListener *core_event_listener;

  //! Did the OS announce a powersave?
  bool powersave;

  //! Time the OS announces a resume from powersave
  time_t powersave_resume_time;

  //! OperationMode before powersave
  OperationMode powersave_operation_mode;

  //! What to do with activity during insisted break?
  ICore::InsistPolicy insist_policy;

  //! Policy currently in effect.
  ICore::InsistPolicy active_insist_policy;

  //! Resumes this break if current break ends.
  BreakId resume_break;

  //! Current local monitor state.
  ActivityState local_state;

  //! Current overall monitor state.
  ActivityState monitor_state;

  //! Current overall monitor state.
  time_t active_since;
  
#ifdef HAVE_DBUS
  //! DBUS bridge
  DBus *dbus;
#endif
  
#ifdef HAVE_TESTS
  //! A fake activity monitor for testing puposes.
  FakeActivityMonitor *fake_monitor;
#endif

#ifdef HAVE_DISTRIBUTION
  Network *network;
#endif

  //! External activity
  std::map<std::string, time_t> external_activity;

#ifdef HAVE_TESTS
  friend class Test;

  // Manual clock
  bool manual_clock;
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

#endif // CORE_HH
