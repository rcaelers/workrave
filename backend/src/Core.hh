// Core.hh --- The main controller
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "config/Config.hh"
#include "dbus/DBus.hh"

#include "ICore.hh"
#include "BreaksControl.hh"
#include "ActivityMonitor.hh"
#include "Statistics.hh"

#if FIXME
#include "Networking.hh"
#endif
#include "CoreHooks.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave {
  class IApp;
}

class Core :
  public ICore,
  public IConfiguratorListener,
  public boost::enable_shared_from_this<Core>
{
public:
  Core(int id = 0);
  virtual ~Core();

  // ICore

  boost::signals2::signal<void(OperationMode)> &signal_operation_mode_changed();
  boost::signals2::signal<void(UsageMode)> &signal_usage_mode_changed();
  
  void init(int argc, char **argv, IApp *application, const std::string &display_name);
  void heartbeat();

  void force_break(BreakId id, BreakHint break_hint);
  IBreak::Ptr get_break(BreakId id);
  IStatistics::Ptr get_statistics() const;
  IConfigurator::Ptr get_configurator() const;
  ICoreHooks::Ptr get_hooks() const;
  dbus::DBus::Ptr get_dbus() const;
  bool is_user_active() const;

  OperationMode get_operation_mode();
  OperationMode get_operation_mode_regular();
  bool is_operation_mode_an_override();
  void set_operation_mode(OperationMode mode);
  void set_operation_mode_override(OperationMode mode, const std::string &id);
  void remove_operation_mode_override(const std::string &id);
 
  UsageMode get_usage_mode();
  void set_usage_mode(UsageMode mode);

  void set_insist_policy(InsistPolicy p);
  
  void force_idle();

  // DBus functions.
  void report_external_activity(std::string who, bool act);
  
private:
  void init_breaks();
  void init_configurator();
  void init_monitor(const std::string &display_name);
  void init_bus();
  void init_statistics();

  void config_changed_notify(const std::string &key);
  void load_config();

  void set_operation_mode_internal(OperationMode mode, bool persistent, const std::string &override_id = "");
  void set_usage_mode_internal(UsageMode mode, bool persistent);
  
private:
  //
  int id;
  
  //! Number of command line arguments passed to the program.
  int argc;

  //! Command line arguments passed to the program.
  char **argv;

  //! List of breaks.
  BreaksControl::Ptr breaks_control;

  //! The Configurator.
  IConfigurator::Ptr configurator;

  //! The activity monitor
  ActivityMonitor::Ptr monitor;

  //! The activity monitor
  CoreHooks::Ptr hooks;

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

  //! DBUS bridge
  dbus::DBus::Ptr dbus;
  
  boost::signals2::signal<void(OperationMode)> operation_mode_changed_signal;
  boost::signals2::signal<void(UsageMode)> usage_mode_changed_signal;
};

#endif // CORE_HH
