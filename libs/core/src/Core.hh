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

#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstddef>
#include <cstdlib>

#ifdef PLATFORM_OS_MACOS
#  include "MacOSHelpers.hh"
#endif

#include <iostream>
#include <string>
#include <map>

#include "Break.hh"
#include "IActivityMonitor.hh"
#include "core/ICore.hh"
#include "core/ICoreEventListener.hh"
#include "config/IConfiguratorListener.hh"
#include "Timer.hh"
#include "Statistics.hh"
#include "utils/Diagnostics.hh"
#include "CoreHooks.hh"
#include "LocalActivityMonitor.hh"

#include "dbus/IDBus.hh"

using namespace workrave;

class Statistics;
class FakeActivityMonitor;
class IdleLogManager;
class BreakControl;

#ifdef HAVE_DISTRIBUTION
#  include "DistributionManager.hh"
#  include "IDistributionClientMessage.hh"
#  include "DistributionListener.hh"
#endif

class Core
  :
#ifdef HAVE_DISTRIBUTION
  public IDistributionClientMessage
  , public DistributionListener
  ,
#endif
  public ICore
  , public workrave::config::IConfiguratorListener
{
public:
  Core();
  ~Core() override;

  static Core *get_instance();
#ifdef HAVE_TESTS
  static void reset_instance();
#endif

  // ICore
  boost::signals2::signal<void(workrave::OperationMode)> &signal_operation_mode_changed() override;
  boost::signals2::signal<void(workrave::UsageMode)> &signal_usage_mode_changed() override;

  Timer *get_timer(std::string name) const;
  Timer *get_timer(BreakId id) const;
  Break *get_break(BreakId id) override;
  Break *get_break(std::string name) override;
  workrave::config::IConfigurator::Ptr get_configurator() const override;
  IActivityMonitor::Ptr get_activity_monitor() const;
  bool is_user_active() const override;
  std::string get_break_stage(BreakId id);

#ifdef HAVE_DISTRIBUTION
  DistributionManager *get_distribution_manager() const override;
#endif
  Statistics *get_statistics() const override;
  void set_core_events_listener(ICoreEventListener *l) override;
  void force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint) override;
  void time_changed() override;
  void set_powersave(bool down) override;

  int64_t get_time() const override;
  void post_event(CoreEvent event) override;

  OperationMode get_operation_mode() override;
  OperationMode get_operation_mode_regular() override;
  bool is_operation_mode_an_override() override;
  void set_operation_mode(OperationMode mode) override;
  void set_operation_mode_override(OperationMode mode, const std::string &id) override;
  void remove_operation_mode_override(const std::string &id) override;

  UsageMode get_usage_mode() override;
  void set_usage_mode(UsageMode mode) override;

  void set_freeze_all_breaks(bool freeze);
  void set_insensitive_mode_all_breaks(InsensitiveMode mode);

  void stop_prelude(BreakId break_id);
  void do_force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint);

  void freeze();
  void defrost();

  void force_idle() override;
  void force_idle(BreakId break_id);

  ActivityState get_current_monitor_state() const;
  bool is_master() const;

  // DBus functions.
  void report_external_activity(std::string who, bool act);
  void is_timer_running(BreakId id, bool &value);
  void get_timer_elapsed(BreakId id, int *value);
  void get_timer_remaining(BreakId id, int *value);
  void get_timer_idle(BreakId id, int *value);
  void get_timer_overdue(BreakId id, int *value);

  void postpone_break(BreakId break_id);
  void skip_break(BreakId break_id);

  workrave::dbus::IDBus::Ptr get_dbus() const override;
  ICoreHooks::Ptr get_hooks() const override;

private:
#ifndef NDEBUG
  enum ScriptCommand
  {
    SCRIPT_START = 1,
  };
#endif

  void init(int argc, char **argv, IApp *application, const char *display_name) override;
  void init_breaks();
  void init_configurator();
  void init_monitor(const char *display_name);
  void init_distribution_manager();
  void init_bus();
  void init_statistics();

  void load_monitor_config();
  void config_changed_notify(const std::string &key) override;
  void heartbeat() override;
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

  void set_insist_policy(InsistPolicy p) override;
  InsistPolicy get_insist_policy() const;

  void set_operation_mode_internal(OperationMode mode, bool persistent, const std::string &override_id = "");
  void set_usage_mode_internal(UsageMode mode, bool persistent);

#ifdef HAVE_DISTRIBUTION
  bool request_client_message(DistributionClientMessageID id, PacketBuffer &buffer) override;
  bool client_message(DistributionClientMessageID id, bool master, const char *client_id, PacketBuffer &buffer) override;

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
  void send_break_control_message_bool_param(BreakId break_id, BreakControlMessage message, bool param);
  bool set_break_control(PacketBuffer &buffer);

  void signon_remote_client(std::string client_id) override;
  void signoff_remote_client(std::string client_id) override;
  void compute_timers();
#endif // HAVE_DISTRIBUTION

private:
  //! The one and only instance
  static Core *instance;

  //! Number of command line arguments passed to the program.
  int argc{};

  //! Command line arguments passed to the program.
  char **argv{};

  //! The time we last processed the timers.
  int64_t last_process_time{0};

  //! Are we the master node??
  TracedField<bool> master_node{"core.master_node", true};

  //! List of breaks.
  Break breaks[workrave::BREAK_ID_SIZEOF];

  //! The Configurator.
  workrave::config::IConfigurator::Ptr configurator{nullptr};

  //! The activity monitor
  IActivityMonitor::Ptr monitor;

  //! The activity monitor
  LocalActivityMonitor::Ptr local_monitor;

  //! GUI Widget factory.
  IApp *application{nullptr};

  //! The statistics collector.
  Statistics *statistics{nullptr};

  //! Current operation mode.
  TracedField<OperationMode> operation_mode{"core.operation_mode", OperationMode::Normal};

  //! The same as operation_mode unless operation_mode is an override mode.
  TracedField<OperationMode> operation_mode_regular{"core.operation_mode_regular", OperationMode::Normal};

  //! Active operation mode overrides.
  std::map<std::string, OperationMode> operation_mode_overrides;

  //! Current usage mode.
  TracedField<UsageMode> usage_mode{"core.usage_mode", UsageMode::Normal};

  //! Where to send core events to?
  ICoreEventListener *core_event_listener{nullptr};

  //! Did the OS announce a powersave?
  TracedField<bool> powersave{"core.powersave", false};

  //! Time the OS announces a resume from powersave
  int64_t powersave_resume_time{0};

  //! What to do with activity during insisted break?
  TracedField<InsistPolicy> insist_policy{"core.insist_policy", InsistPolicy::Halt};

  //! Policy currently in effect.
  TracedField<InsistPolicy> active_insist_policy{"core.active_insist_policy", InsistPolicy::Invalid};

  //! Resumes this break if current break ends.
  TracedField<BreakId> resume_break{"core.resume_break", BREAK_ID_NONE};

  //! Current local monitor state.
  TracedField<ActivityState> local_state{"core.local_state", ACTIVITY_IDLE};

  //! Current overall monitor state.
  TracedField<ActivityState> monitor_state{"core.monitor_state", ACTIVITY_UNKNOWN};

  //! DBUS bridge
  workrave::dbus::IDBus::Ptr dbus;

  //! Hooks to alter the backend behaviour.
  CoreHooks::Ptr hooks;

#ifdef HAVE_DISTRIBUTION
  //! The Distribution Manager
  DistributionManager *dist_manager{nullptr};

  //! State of the remote master.
  TracedField<ActivityState> remote_state{"core.remote_state", ACTIVITY_IDLE};

  //! Manager that collects idle times of all clients.
  IdleLogManager *idlelog_manager{nullptr};

#  ifndef NDEBUG
  //! A fake activity monitor for testing puposes.
  FakeActivityMonitor *fake_monitor{nullptr};
#  endif
#endif

  //! External activity
  std::map<std::string, int64_t> external_activity;

  //! Operation mode changed notification.
  boost::signals2::signal<void(workrave::OperationMode)> operation_mode_changed_signal;

  //! Usage mode changed notification.
  boost::signals2::signal<void(workrave::UsageMode)> usage_mode_changed_signal;

#ifdef HAVE_TESTS
  friend class Test;
#endif
};

//! Returns the singleton Core instance.
inline Core *
Core::get_instance()
{
  if (instance == nullptr)
    {
      instance = new Core();
    }

  return instance;
}

#ifdef HAVE_TESTS
inline void
Core::reset_instance()
{
  if (instance != nullptr)
    {
      delete instance;
      instance = nullptr;
    }
}
#endif

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
