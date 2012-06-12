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

#include <string>
#include <map>

#include <boost/enable_shared_from_this.hpp>

#include "utils/ITimeSource.hh"
#include "config/Config.hh"

#include "ICore.hh"
#include "IBreakSupport.hh"
#include "Break.hh"
#include "ActivityMonitor.hh"
#include "Timer.hh"
#include "Statistics.hh"

#include "Networking.hh"

using namespace workrave;
using namespace workrave::utils;

// Forward declarion of external interface.
namespace workrave {
  class IApp;
  namespace dbus
  {
    class DBus;
  }
}

class Core :
  public ITimeSource,
  public ICore,
  public IBreakSupport,
  public IConfiguratorListener,
  public boost::enable_shared_from_this<Core>
{
public:
  Core();
  virtual ~Core();

  // ITimeSource
  
  time_t get_time() const;

  // ICore


#ifdef HAVE_DBUS
  dbus::DBus *get_dbus()
  {
    return dbus;
  }
#endif


  boost::signals2::signal<void(OperationMode)> &signal_operation_mode_changed();
  boost::signals2::signal<void(UsageMode)> &signal_usage_mode_changed();
  
  void init(int argc, char **argv, IApp *application, const std::string &display_name);
  void heartbeat();

  void force_break(BreakId id, BreakHint break_hint);
  IBreak::Ptr get_break(BreakId id);
  IStatistics::Ptr get_statistics() const;
  IConfigurator::Ptr get_configurator() const;
  bool is_user_active() const;

  OperationMode get_operation_mode();
  OperationMode get_operation_mode_regular();
  bool is_operation_mode_an_override();
  void set_operation_mode(OperationMode mode);
  void set_operation_mode_override(OperationMode mode, const std::string &id);
  void remove_operation_mode_override(const std::string &id);

  UsageMode get_usage_mode();
  void set_usage_mode(UsageMode mode);

  void set_powersave(bool down);
  void time_changed();

  void set_insist_policy(ICore::InsistPolicy p);
  
  void force_idle();

  // ICoreInternal
  //IActivityMonitor::Ptr get_activity_monitor() const;
  // Break::Ptr get_break(std::string name);
  void resume_reading_mode_timers();
  void defrost();
  void freeze();
  void force_break_idle(BreakId break_id);
  IActivityMonitor::Ptr create_timer_activity_monitor(const string &break_name);

  // DBus functions.
  void report_external_activity(std::string who, bool act);
  // void is_timer_running(BreakId id, bool &value);
  // void get_timer_elapsed(BreakId id,int *value);
  // void get_timer_idle(BreakId id, int *value);
  // void get_timer_overdue(BreakId id,int *value);
  // void postpone_break(BreakId break_id);
  // void skip_break(BreakId break_id);

  
private:
  void init_breaks();
  void init_configurator();
  void init_monitor(const std::string &display_name);
  void init_bus();
  void init_statistics();

  void config_changed_notify(const std::string &key);
  void process_state();
  bool process_timewarp();
  void process_timers();
  void start_break(BreakId break_id, BreakId resume_this_break = BREAK_ID_NONE);
  void stop_all_breaks();
  void daily_reset();
  void save_state() const;
  void load_state();
  void load_misc();

  void set_operation_mode_internal(OperationMode mode, bool persistent, const std::string &override_id = "");
  void set_usage_mode_internal(UsageMode mode, bool persistent);
  void set_freeze_all_breaks(bool freeze);

  Timer::Ptr get_timer(std::string name) const;
  Timer::Ptr get_timer(int id) const;
  
private:
  //! Number of command line arguments passed to the program.
  int argc;

  //! Command line arguments passed to the program.
  char **argv;

  //! The current time.
  time_t current_time;

  //! The time we last processed the timers.
  time_t last_process_time;

  //! List of breaks.
  Break::Ptr breaks[BREAK_ID_SIZEOF];

  //! The Configurator.
  IConfigurator::Ptr configurator;

  //! The activity monitor
  ActivityMonitor::Ptr monitor;

  //! GUI Widget factory.
  IApp *application;

  //! The statistics collector.
  Statistics::Ptr statistics;

  //! Current operation mode.
  OperationMode operation_mode;

  //! The same as operation_mode unless operation_mode is an override mode.
  OperationMode operation_mode_regular;

  //! Active operation mode overrides.
  std::map<std::string, OperationMode> operation_mode_overrides;

  //! Current usage mode.
  UsageMode usage_mode;

  //! Did the OS announce a powersave?
  bool powersave;

  //! Time the OS announces a resume from powersave
  time_t powersave_resume_time;

  //! What to do with activity during insisted break?
  ICore::InsistPolicy insist_policy;

  //! Policy currently in effect.
  ICore::InsistPolicy active_insist_policy;

  //! Current overall monitor state.
  ActivityState monitor_state;

#ifdef HAVE_DBUS
  //! DBUS bridge
  dbus::DBus *dbus;
#endif
  
  //! External activity
  std::map<std::string, time_t> external_activity;

  boost::signals2::signal<void(OperationMode)> operation_mode_changed_signal;
  boost::signals2::signal<void(UsageMode)> usage_mode_changed_signal;
};

#endif // CORE_HH
