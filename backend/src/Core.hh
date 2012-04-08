// Core.hh --- The main controller
//
// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "IDistributionClientMessage.hh"
#include "DistributionListener.hh"
#endif

class Core :
#ifdef HAVE_DISTRIBUTION
  public IDistributionClientMessage,
  public DistributionListener,
#endif
  public TimeSource,
  public ICore,
  public IConfiguratorListener,
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

#ifdef HAVE_DISTRIBUTION
  DistributionManager *get_distribution_manager() const;
#endif
  Statistics *get_statistics() const;
  void set_core_events_listener(ICoreEventListener *l);
  void force_break(BreakId id, BreakHint break_hint);
  void time_changed();
  void set_powersave(bool down);

  time_t get_time() const;
  void post_event(CoreEvent event);

  OperationMode get_operation_mode();
  OperationMode get_operation_mode_regular();
  bool is_operation_mode_an_override();
  void set_operation_mode(OperationMode mode);
  void set_operation_mode_override( OperationMode mode, const std::string &id );
  void remove_operation_mode_override( const std::string &id );

  UsageMode get_usage_mode();
  void set_usage_mode(UsageMode mode);

  void set_freeze_all_breaks(bool freeze);

  void stop_prelude(BreakId break_id);
  void do_force_break(BreakId id, BreakHint break_hint);

  void freeze();
  void defrost();

  void force_idle();
  void force_idle(BreakId break_id);

  ActivityState get_current_monitor_state() const;
  bool is_master() const;

  // DBus functions.
  void report_external_activity(std::string who, bool act);
  void is_timer_running(BreakId id, bool &value);
  void get_timer_elapsed(BreakId id,int *value);
  void get_timer_idle(BreakId id, int *value);
  void get_timer_overdue(BreakId id,int *value);

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
  void init_distribution_manager();
  void init_bus();
  void init_statistics();

  void load_monitor_config();
  void config_changed_notify(const std::string &key);
  void heartbeat();
  void timer_action(BreakId id, TimerInfo info);
  void process_distribution();
  void process_state();
  bool process_timewarp();
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

  void set_operation_mode_internal(OperationMode mode, bool persistent, const std::string &override_id = "");
  void set_usage_mode_internal(UsageMode mode, bool persistent);
  
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
#endif // HAVE_DISTRIBUTION


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
  IApp *application;

  //! The statistics collector.
  Statistics *statistics;

  //! Current operation mode.
  OperationMode operation_mode;

  //! The same as operation_mode unless operation_mode is an override mode.
  OperationMode operation_mode_regular;

  //! Active operation mode overrides.
  std::map<std::string, OperationMode> operation_mode_overrides;

  //! Current usage mode.
  UsageMode usage_mode;

  //! Where to send core events to?
  ICoreEventListener *core_event_listener;

  //! Did the OS announce a powersave?
  bool powersave;

  //! Time the OS announces a resume from powersave
  time_t powersave_resume_time;

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

#ifdef HAVE_DBUS
  //! DBUS bridge
  DBus *dbus;
#endif

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
#endif
#endif

  //! External activity
  std::map<std::string, time_t> external_activity;

#ifdef HAVE_TESTS
  friend class Test;
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
