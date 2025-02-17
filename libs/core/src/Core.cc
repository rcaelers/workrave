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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include "debug.hh"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>

#include <spdlog/spdlog.h>

#include "Core.hh"

#include "core/IApp.hh"
#include "core/ICoreEventListener.hh"
#include "LocalActivityMonitor.hh"
#include "TimerActivityMonitor.hh"
#include "Break.hh"
#include "config/ConfiguratorFactory.hh"
#include "config/IConfigurator.hh"
#include "core/CoreConfig.hh"
#include "Statistics.hh"
#include "BreakControl.hh"
#include "Timer.hh"
#include "TimePredFactory.hh"
#include "TimePred.hh"
#include "utils/TimeSource.hh"
#include "input-monitor/InputMonitorFactory.hh"
#include "utils/AssetPath.hh"
#include "utils/Paths.hh"

#include "dbus/DBusFactory.hh"
#if defined(PLATFORM_OS_WINDOWS_NATIVE)
#  undef interface
#endif
#include "dbus/IDBus.hh"
#include "dbus/DBusException.hh"
#if defined(HAVE_DBUS)
#  include "DBusWorkrave.hh"
#endif
#if defined(HAVE_TESTS)
#  include "Test.hh"
#endif

Core *Core::instance = nullptr;
workrave::config::IConfigurator::Ptr Core::configurator = nullptr;

const char *WORKRAVESTATE = "WorkRaveState";
const int SAVESTATETIME = 60;

#define DBUS_PATH_WORKRAVE "/org/workrave/Workrave/Core"
#define DBUS_SERVICE_WORKRAVE "org.workrave.Workrave"

using namespace workrave::utils;
using namespace workrave::config;
using namespace std;

ICore::Ptr
CoreFactory::create(workrave::config::IConfigurator::Ptr configurator)
{
  Core::set_configurator(configurator);
  return std::make_shared<Core>();
}

void
Core::set_configurator(workrave::config::IConfigurator::Ptr configurator)
{
  Core::configurator = configurator;
}

//! Constructs a new Core.
Core::Core()
{
  TRACE_ENTRY();
  hooks = std::make_shared<CoreHooks>();
  TimeSource::sync();

  assert(!instance);
  instance = this;
}

//! Destructor.
Core::~Core()
{
  TRACE_ENTRY();
  save_state();

  if (monitor != nullptr)
    {
      monitor->terminate();
    }

  delete statistics;
}

/********************************************************************************/
/**** Initialization                                                       ******/
/********************************************************************************/

//! Initializes the core.
void
Core::init(int argc, char **argv, IApp *app, const char *display_name)
{
  application = app;
  this->argc = argc;
  this->argv = argv;

  CoreConfig::init(configurator);

  init_monitor(display_name);

  init_breaks();
  init_statistics();
  init_bus();

  load_state();
  load_misc();
}

//! Initializes the communication bus.
void
Core::init_bus()
{
  TRACE_ENTRY();
  try
    {
      dbus = workrave::dbus::DBusFactory::create();
      dbus->init();

#if defined(HAVE_DBUS)
      extern void init_DBusWorkrave(std::shared_ptr<workrave::dbus::IDBus> dbus);
      init_DBusWorkrave(dbus);
#endif

      dbus->register_object_path(DBUS_PATH_WORKRAVE);
      dbus->connect(DBUS_PATH_WORKRAVE, "org.workrave.CoreInterface", this);
      dbus->connect(DBUS_PATH_WORKRAVE, "org.workrave.ConfigInterface", configurator.get());

#if defined(HAVE_TESTS)
      dbus->connect("/org/workrave/Workrave/Debug", "org.workrave.DebugInterface", Test::get_instance());
      dbus->register_object_path("/org/workrave/Workrave/Debug");
#endif
    }
  catch (workrave::dbus::DBusException &)
    {
      TRACE_MSG("Ex!");
    }
}

//! Initializes the activity monitor.
void
Core::init_monitor(const char *display_name)
{
  workrave::input_monitor::InputMonitorFactory::init(configurator, display_name);

  configurator->set_value(CoreConfig::CFG_KEY_MONITOR_SENSITIVITY, 3, workrave::config::CONFIG_FLAG_INITIAL);

  local_monitor = std::make_shared<LocalActivityMonitor>();

#if defined(HAVE_TESTS)
  if (hooks->hook_create_monitor())
    {
      monitor = hooks->hook_create_monitor()();
    }
  else
#endif
    {
      monitor = local_monitor;
    }

  load_monitor_config();

  configurator->add_listener(CoreConfig::CFG_KEY_MONITOR, this);
}

//! Initializes all breaks.
void
Core::init_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i].init(BreakId(i), configurator, application);
    }
}

//! Initializes the statistics.
void
Core::init_statistics()
{
  statistics = new Statistics();
  statistics->init(this);
}

//! Loads the configuration of the monitor.
void
Core::load_monitor_config()
{
  TRACE_ENTRY();
  int noise;
  int activity;
  int idle;
  int sensitivity;

  assert(configurator != nullptr);
  assert(local_monitor != nullptr);

  if (!configurator->get_value(CoreConfig::CFG_KEY_MONITOR_NOISE, noise))
    {
      noise = 9000;
    }
  if (!configurator->get_value(CoreConfig::CFG_KEY_MONITOR_ACTIVITY, activity))
    {
      activity = 1000;
    }
  if (!configurator->get_value(CoreConfig::CFG_KEY_MONITOR_IDLE, idle))
    {
      idle = 5000;
    }
  if (!configurator->get_value(CoreConfig::CFG_KEY_MONITOR_SENSITIVITY, sensitivity))
    {
      sensitivity = 3;
    }

  // Pre 1.0 compatibility...
  if (noise < 50)
    {
      noise *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_NOISE, noise);
    }

  if (activity < 50)
    {
      activity *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_ACTIVITY, activity);
    }

  if (idle < 50)
    {
      idle *= 1000;
      configurator->set_value(CoreConfig::CFG_KEY_MONITOR_IDLE, idle);
    }

  TRACE_MSG("Monitor config = {} {} {}", noise, activity, idle);

  local_monitor->set_parameters(noise, activity, idle, sensitivity);
}

//! Notification that the configuration has changed.
void
Core::config_changed_notify(const string &key)
{
  TRACE_ENTRY_PAR(key);
  string::size_type pos = key.find('/');
  string path;

  if (pos != string::npos)
    {
      path = key.substr(0, pos);
    }

  if (path == CoreConfig::CFG_KEY_MONITOR)
    {
      load_monitor_config();
    }

  if (key == CoreConfig::CFG_KEY_OPERATION_MODE)
    {
      int mode;
      if (!configurator->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
        {
          mode = underlying_cast(OperationMode::Normal);
        }
      if (!workrave::utils::enum_in_range<OperationMode>(mode))
        {
          mode = underlying_cast(OperationMode::Normal);
        }
      TRACE_MSG("Setting operation mode");
      set_operation_mode_internal(OperationMode(mode));
    }

  if (key == CoreConfig::CFG_KEY_USAGE_MODE)
    {
      int mode;
      if (!configurator->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
        {
          mode = underlying_cast(UsageMode::Normal);
        }
      if (!workrave::utils::enum_in_range<UsageMode>(mode))
        {
          mode = underlying_cast(UsageMode::Normal);
        }
      TRACE_MSG("Setting usage mode");
      set_usage_mode_internal(UsageMode(mode), false);
    }
}

/********************************************************************************/
/**** TimeSource interface                                                 ******/
/********************************************************************************/

int64_t
Core::get_time() const
{
  return TimeSource::get_real_time_sec();
}

/********************************************************************************/
/**** Core Interface                                                       ******/
/********************************************************************************/

//! Returns the specified timer.
Timer *
Core::get_timer(BreakId id) const
{
  if (id >= 0 && id < BREAK_ID_SIZEOF)
    {
      return breaks[id].get_timer();
    }
  else
    {
      return nullptr;
    }
}

//! Returns the specified timer.
Timer *
Core::get_timer(string name) const
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_name() == name)
        {
          return breaks[i].get_timer();
        }
    }
  return nullptr;
}

//!
ICoreHooks::Ptr
Core::get_hooks() const
{
  return hooks;
}

std::shared_ptr<workrave::dbus::IDBus>
Core::get_dbus() const
{
  return dbus;
}

//! Returns the activity monitor.
IActivityMonitor::Ptr
Core::get_activity_monitor() const
{
  return monitor;
}

//! Returns the statistics.
Statistics *
Core::get_statistics() const
{
  return statistics;
}

//! Returns the specified break controller.
Break *
Core::get_break(BreakId id)
{
  assert(id >= 0 && id < BREAK_ID_SIZEOF);
  return &breaks[id];
}

//! Returns the specified break controller.
Break *
Core::get_break(std::string name)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_name() == name)
        {
          return &breaks[i];
        }
    }
  return nullptr;
}

std::string
Core::get_break_stage(BreakId id)
{
  if (id >= 0 && id < BREAK_ID_SIZEOF)
    {
      return breaks[id].get_break_control()->get_current_stage();
    }
  else
    {
      return "";
    }
}

//! Retrieves the operation mode.
OperationMode
Core::get_active_operation_mode()
{
  return operation_mode_active;
}

//! Retrieves the regular operation mode.
OperationMode
Core::get_regular_operation_mode()
{
  /* operation_mode_regular is the same as operation_mode unless there's an
  override in place, in which case operation_mode is the current override mode and
  operation_mode_regular is the mode that will be restored once all overrides are removed
  */
  return operation_mode_regular;
}

//! Checks if operation_mode is an override.
bool
Core::is_operation_mode_an_override()
{
  return !operation_mode_overrides.empty();
}

void
Core::set_operation_mode(OperationMode mode)
{
  using namespace std::chrono_literals;

  set_operation_mode_internal(mode);
  CoreConfig::operation_mode_auto_reset_duration().set(0min);
  CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
}

void
Core::set_operation_mode_for(OperationMode mode, std::chrono::minutes duration)
{
  using namespace std::chrono_literals;

  set_operation_mode_internal(mode);
  CoreConfig::operation_mode_auto_reset_duration().set(duration);
  if (duration > 0min)
    {
      CoreConfig::operation_mode_auto_reset_time().set(workrave::utils::TimeSource::get_real_time() + duration);
    }
  else
    {
      CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
    }
}

//! Temporarily overrides the operation mode.
void
Core::set_operation_mode_override(OperationMode mode, const std::string &id)
{
  if (!id.empty())
    {
      operation_mode_overrides[id] = mode;
      update_active_operation_mode();
    }
}

//! Removes the overridden operation mode.
void
Core::remove_operation_mode_override(const std::string &id)
{
  if (!id.empty() && !operation_mode_overrides.empty())
    {
      operation_mode_overrides.erase(id);
      update_active_operation_mode();
    }
}

//! Set the operation mode.
void
Core::set_operation_mode_internal(OperationMode mode)
{
  if (operation_mode_regular != mode)
    {
      operation_mode_regular = mode;
      update_active_operation_mode();
      CoreConfig::operation_mode().set(mode);
      operation_mode_changed_signal(operation_mode_regular);

#if defined(HAVE_DBUS)
      org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
      if (iface != nullptr)
        {
          iface->OperationModeChanged("/org/workrave/Workrave/Core", operation_mode_regular);
        }
#endif
    }
}

void
Core::update_active_operation_mode()
{
  // std::cout << "Core::update_active_operation_mode " << operation_mode_regular << " " << operation_mode_active << " "
  //           << operation_mode_overrides.size() << std::endl;
  OperationMode mode = operation_mode_regular;

  /* Find the most important override. Override modes in order of importance:
     OperationMode::Suspended, OperationMode::Quiet, OperationMode::Normal
  */
  for (auto i = operation_mode_overrides.begin(); (i != operation_mode_overrides.end()); ++i)
    {
      if (i->second == OperationMode::Suspended)
        {
          mode = OperationMode::Suspended;
          break;
        }

      if ((i->second == OperationMode::Quiet) && (mode == OperationMode::Normal))
        {
          mode = OperationMode::Quiet;
        }
    }
  // std::cout << "Core::update_active_operation_mode " << operation_mode_regular << " " << operation_mode_active << " " << mode
  //           << " " << operation_mode_overrides.size() << std::endl;
  if (operation_mode_active != mode)
    {
      OperationMode previous_mode = operation_mode_active;
      operation_mode_active = mode;

      if (operation_mode_active == OperationMode::Suspended)
        {
          force_idle();
          monitor->suspend();
          stop_all_breaks();

          for (int i = 0; i < BREAK_ID_SIZEOF; ++i)
            {
              if (breaks[i].is_enabled())
                {
                  breaks[i].get_timer()->set_insensitive_mode(INSENSITIVE_MODE_IDLE_ALWAYS);
                }
            }
        }
      else if (previous_mode == OperationMode::Suspended)
        {
          // stop_all_breaks again will reset insensitive mode (that is good)
          stop_all_breaks();
          monitor->resume();
        }

      if (operation_mode_active == OperationMode::Quiet)
        {
          stop_all_breaks();
        }

      // TODO: check if needed.
      // TODO: check also missing in corenext
      // operation_mode_changed_signal(operation_mode_active);
    }
}

void
Core::check_operation_mode_auto_reset()
{
  auto next_reset_time = CoreConfig::operation_mode_auto_reset_time()();

  if ((next_reset_time.time_since_epoch().count() > 0) && (workrave::utils::TimeSource::get_real_time() >= next_reset_time)
      && (CoreConfig::operation_mode()() != OperationMode::Normal))
    {
      spdlog::debug("Resetting operation mode");
      set_operation_mode(OperationMode::Normal);
    }
}

//! Retrieves the usage mode.
UsageMode
Core::get_usage_mode()
{
  return usage_mode;
}

//! Sets the usage mode.
void
Core::set_usage_mode(UsageMode mode)
{
  set_usage_mode_internal(mode, true);
}

//! Sets the usage mode.
void
Core::set_usage_mode_internal(UsageMode mode, bool persistent)
{
  if (usage_mode != mode)
    {
      usage_mode = mode;

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          breaks[i].set_usage_mode(mode);
        }

      if (persistent)
        {
          configurator->set_value(CoreConfig::CFG_KEY_USAGE_MODE, underlying_cast(mode));
        }

      usage_mode_changed_signal(mode);

#if defined(HAVE_DBUS)
      org_workrave_CoreInterface *iface = org_workrave_CoreInterface::instance(dbus);
      if (iface != nullptr)
        {
          iface->UsageModeChanged("/org/workrave/Workrave/Core", mode);
        }
#endif
    }
}

//! Sets the listener for core events.
void
Core::set_core_events_listener(ICoreEventListener *l)
{
  core_event_listener = l;
}

//! Forces the start of the specified break.
void
Core::force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint)
{
  do_force_break(id, break_hint);
}

//! Forces the start of the specified break.
void
Core::do_force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint)
{
  TRACE_ENTRY_PAR(id);
  BreakControl *microbreak_control = breaks[BREAK_ID_MICRO_BREAK].get_break_control();
  BreakControl *breaker = breaks[id].get_break_control();

  if (id == BREAK_ID_REST_BREAK && (microbreak_control->get_break_state() == BreakControl::BREAK_ACTIVE))
    {
      microbreak_control->stop_break(false);
      resume_break = BREAK_ID_MICRO_BREAK;
      TRACE_MSG("Resuming Micro break");
    }

  breaker->force_start_break(break_hint);
}

//! Announces a change in time.
void
Core::time_changed()
{
  TRACE_ENTRY();
  // In case out timezone changed..
  tzset();

  // A change of system time idle handled by process_timewarp.
  // This is used to handle a change in timezone on windows.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      breaks[i].get_timer()->shift_time(0);
    }
}

//! Announces a powersave state.
void
Core::set_powersave(bool down)
{
  TRACE_ENTRY_PAR(down);
  TRACE_VAR(powersave, powersave_resume_time, operation_mode_active);

  if (down)
    {
      if (!powersave)
        {
          // Computer is going down
          set_operation_mode_override(OperationMode::Suspended, "powersave");
          powersave_resume_time = 0;
          powersave = true;
        }

      for (const auto &i: breaks)
        {
          i.get_timer()->stop_timer();
        }

      save_state();
      statistics->update();
    }
  else
    {
      // Computer is coming back
      // leave powersave true until the timewarp is detected
      // or until some time has passed
      if (powersave_resume_time == 0)
        {
          int64_t current_time = TimeSource::get_real_time_sec();

          powersave_resume_time = current_time ? current_time : 1;
          TRACE_MSG("set resume time {}", powersave_resume_time);
        }

      TRACE_MSG("resume time {}", powersave_resume_time);
      remove_operation_mode_override("powersave");
    }
}

//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
Core::set_insist_policy(InsistPolicy p)
{
  TRACE_ENTRY_PAR(p);

  if (active_insist_policy != InsistPolicy::Invalid && insist_policy != p)
    {
      TRACE_MSG("refreeze {}", active_insist_policy);
      defrost();
      insist_policy = p;
      freeze();
    }
  else
    {
      insist_policy = p;
    }
}

//! Gets the insist policy.
InsistPolicy
Core::get_insist_policy() const
{
  return insist_policy;
}

// ! Forces all monitors to be idle.
void
Core::force_idle()
{
  TRACE_ENTRY();
  force_idle(BREAK_ID_NONE);
}

void
Core::force_idle(BreakId break_id)
{
  TRACE_ENTRY();
  monitor->force_idle();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (break_id == BREAK_ID_NONE || i == break_id)
        {
          IActivityMonitor *am = breaks[i].get_timer()->get_activity_monitor();
          if (am != nullptr)
            {
              am->force_idle();
            }
        }

      breaks[i].get_timer()->force_idle();
    }
}

/********************************************************************************/
/**** Break Response                                                       ******/
/********************************************************************************/

//! User postpones the specified break.
void
Core::postpone_break(BreakId break_id)
{
  do_postpone_break(break_id);
}

//! User skips the specified break.
void
Core::skip_break(BreakId break_id)
{
  do_skip_break(break_id);
}

//! User stops the prelude.
void
Core::stop_prelude(BreakId break_id)
{
  TRACE_ENTRY_PAR(break_id);
  do_stop_prelude(break_id);
}

//! User postpones the specified break.
void
Core::do_postpone_break(BreakId break_id)
{
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->postpone_break();
    }
}

//! User skips the specified break.
void
Core::do_skip_break(BreakId break_id)
{
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->skip_break();
    }
}

//!
void
Core::do_stop_prelude(BreakId break_id)
{
  TRACE_ENTRY_PAR(break_id);
  if (break_id >= 0 && break_id < BREAK_ID_SIZEOF)
    {
      BreakControl *bc = breaks[break_id].get_break_control();
      bc->stop_prelude();
    }
}

/********************************************************************************/
/**** Break handling                                                       ******/
/********************************************************************************/

//! Periodic heartbeat.
void
Core::heartbeat()
{
  TRACE_ENTRY();
  assert(application != nullptr);

  TimeSource::sync();

  check_operation_mode_auto_reset();

  // Performs timewarp checking.
  bool warped = process_timewarp();

  // Process configuration
  configurator->heartbeat();

  // Perform distribution processing.
  process_distribution();

  if (!warped)
    {
      // Perform state computation.
      process_state();
    }

  // Perform timer processing.
  process_timers();

  // Send heartbeats to other components.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bc = breaks[i].get_break_control();
      if (bc != nullptr && bc->need_heartbeat())
        {
          bc->heartbeat();
        }
    }

  // Set current time.
  int64_t current_time = TimeSource::get_real_time_sec();

  // Make state persistent.
  if (current_time % SAVESTATETIME == 0)
    {
      statistics->update();
      save_state();
    }

  // Done.
  last_process_time = current_time;
}

//! Performs all distribution processing.
void
Core::process_distribution()
{
  // Default
  master_node = true;
}

//! Computes the current state.
void
Core::process_state()
{
  // Set current time.
  int64_t current_time = TimeSource::get_real_time_sec();

  // Default
  local_state = monitor->get_current_state();

  auto i = external_activity.begin();
  while (i != external_activity.end())
    {
      auto next = i;
      next++;

      if (i->second >= current_time)
        {
          local_state = ACTIVITY_ACTIVE;
        }
      else
        {
          external_activity.erase(i);
        }

      i = next;
    }

  monitor_state = local_state;
}

void
Core::report_external_activity(std::string who, bool act)
{
  TRACE_ENTRY_PAR(who, act);
  if (act)
    {
      int64_t current_time = TimeSource::get_real_time_sec();
      external_activity[who] = current_time + 10;
    }
  else
    {
      external_activity.erase(who);
    }
}

void
Core::is_timer_running(BreakId id, bool &value)
{
  Timer *timer = get_timer(id);
  value = timer->get_state() == STATE_RUNNING;
}

void
Core::get_timer_idle(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int)timer->get_elapsed_idle_time();
}

void
Core::get_timer_elapsed(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int)timer->get_elapsed_time();
}

void
Core::get_timer_remaining(BreakId id, int *value)
{
  Timer *timer = get_timer(id);

  *value = -1;

  if (timer->is_limit_enabled())
    {
      int64_t remaining = timer->get_limit() - timer->get_elapsed_time();
      *value = remaining >= 0 ? remaining : 0;
    }
}

void
Core::get_timer_overdue(BreakId id, int *value)
{
  Timer *timer = get_timer(id);
  *value = (int)timer->get_total_overdue_time();
}

//! Processes all timers.
void
Core::process_timers()
{
  TRACE_ENTRY();
  TimerInfo infos[BREAK_ID_SIZEOF];

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *timer = breaks[i].get_timer();

      infos[i].enabled = breaks[i].is_enabled();
      if (infos[i].enabled)
        {
          timer->enable();
          if (i == BREAK_ID_DAILY_LIMIT)
            {
              timer->set_limit_enabled(timer->get_limit() > 0);
            }
        }
      else
        {
          if (i != BREAK_ID_DAILY_LIMIT)
            {
              timer->disable();
            }
          else
            {
              timer->set_limit_enabled(false);
            }
        }

      // First process only timer that do not have their
      // own activity monitor.
      if (!(timer->has_activity_monitor()))
        {
          timer->process(monitor_state, infos[i]);
        }
    }

  // And process timer with activity monitor.
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (breaks[i].get_timer()->has_activity_monitor())
        {
          breaks[i].get_timer()->process(monitor_state, infos[i]);
        }
    }

  // Process all timer events.
  for (int i = BREAK_ID_SIZEOF - 1; i >= 0; i--)
    {
      TimerInfo &info = infos[i];
      if (breaks[i].is_enabled())
        {
          timer_action((BreakId)i, info);
        }

      if (i == BREAK_ID_DAILY_LIMIT && (info.event == TIMER_EVENT_NATURAL_RESET || info.event == TIMER_EVENT_RESET))
        {
          statistics->set_counter(Statistics::STATS_VALUE_TOTAL_ACTIVE_TIME, (int)info.elapsed_time);
          statistics->start_new_day();

          daily_reset();
        }
    }
}

#if defined(PLATFORM_OS_WINDOWS)

//! Process a possible timewarp on Win32
bool
Core::process_timewarp()
{
  bool ret = false;

  TRACE_ENTRY();
  if (last_process_time != 0)
    {
      int64_t current_time = TimeSource::get_real_time_sec();
      int64_t gap = current_time - 1 - last_process_time;

      if (abs((int)gap) > 5)
        {
          TRACE_MSG("gap {} {} {} {} {}", gap, powersave, operation_mode_active, powersave_resume_time, current_time);
          if (!powersave)
            {
              TRACE_MSG("Time warp of {} seconds. Correcting ", gap);

              force_idle();

              local_monitor->shift_time((int)gap);
              for (int i = 0; i < BREAK_ID_SIZEOF; i++)
                {
                  breaks[i].get_timer()->shift_time((int)gap);
                }

              monitor_state = ACTIVITY_IDLE;
              ret = true;
            }
          else
            {
              TRACE_MSG("Time warp of {} seconds because of powersave", gap);

              force_idle();

              TimeSource::set_real_time_sec_sync(last_process_time + 1);
              monitor_state = ACTIVITY_IDLE;

              process_timers();

              TimeSource::sync();

              // In case the windows message was lost. some people reported that
              // workrave never restarted the timers...
              remove_operation_mode_override("powersave");
            }
        }

      if (powersave && powersave_resume_time != 0 && current_time > powersave_resume_time + 30)
        {
          TRACE_MSG("End of time warp after powersave");

          powersave = false;
          powersave_resume_time = 0;
        }
    }
  return ret;
}

#else

//! Process a possible timewarp On Non-Windows
bool
Core::process_timewarp()
{
  bool ret = false;
  int64_t current_time = TimeSource::get_real_time_sec();

  TRACE_ENTRY();
  if (last_process_time != 0)
    {
      int64_t gap = current_time - 1 - last_process_time;

      if (gap >= 10)
        {
          spdlog::info("Time warp of {} seconds. Powersave", gap);
          TRACE_MSG("current time {}", current_time);
          TRACE_MSG("synced time  {}", TimeSource::get_real_time_sec_sync());
          TRACE_MSG("last process time {}", last_process_time);

          force_idle();

          TimeSource::set_real_time_sec_sync(last_process_time + 1);
          monitor_state = ACTIVITY_IDLE;

          process_timers();

          TimeSource::sync();
          ret = true;

          remove_operation_mode_override("powersave");
        }

      if (powersave && powersave_resume_time != 0 && current_time > powersave_resume_time + 30)
        {
          spdlog::info("End of time warp after powersave");

          powersave = false;
          powersave_resume_time = 0;
        }
    }

  return ret;
}

#endif

//! Notification of a timer action.
/*!
 *  \param timerId ID of the timer that caused the action.
 *  \param action action that is performed by the timer.
 */
void
Core::timer_action(BreakId id, TimerInfo info)
{
  // No breaks when mode is quiet,
  if (operation_mode_active == OperationMode::Quiet && info.event == TIMER_EVENT_LIMIT_REACHED)
    {
      return;
    }

  BreakControl *breaker = breaks[id].get_break_control();

  assert(breaker != nullptr);

  switch (info.event)
    {
    case TIMER_EVENT_LIMIT_REACHED:
      if (breaker->get_break_state() == BreakControl::BREAK_INACTIVE)
        {
          start_break(id);
        }
      break;

    case TIMER_EVENT_NATURAL_RESET:
      statistics->increment_break_counter(id, Statistics::STATS_BREAKVALUE_NATURAL_TAKEN);
      // FALLTHROUGH

    case TIMER_EVENT_RESET:
      if (breaker->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaker->stop_break();
        }
      break;

    default:
      break;
    }
}

//! starts the specified break.
/*!
 *  \param break_id ID of the timer that caused the break.
 */
void
Core::start_break(BreakId break_id, BreakId resume_this_break)
{
  // Don't show MB when RB is active, RB when DL is active.
  for (int bi = break_id; bi <= BREAK_ID_DAILY_LIMIT; bi++)
    {
      if (breaks[bi].get_break_control()->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          return;
        }
    }

  // Advance restbreak if it follows within 30s after the end of a microbreak
  if (break_id == BREAK_ID_REST_BREAK && resume_this_break == BREAK_ID_NONE)
    {
      breaks[BREAK_ID_REST_BREAK].override(BREAK_ID_REST_BREAK);
    }

  if (break_id == BREAK_ID_MICRO_BREAK && breaks[BREAK_ID_REST_BREAK].is_enabled())
    {
      Timer *rb_timer = breaks[BREAK_ID_REST_BREAK].get_timer();
      assert(rb_timer != nullptr);

      bool activity_sensitive = breaks[BREAK_ID_REST_BREAK].get_timer_activity_sensitive();

      // Only advance when
      // 0. It is activity sensitive
      // 1. we have a next limit reached time.
      if (activity_sensitive && rb_timer->get_next_limit_time() > 0)
        {
          Timer *timer = breaks[break_id].get_timer();

          int64_t duration = timer->get_auto_reset();
          int64_t now = TimeSource::get_real_time_sec();

          if (now + duration + 30 >= rb_timer->get_next_limit_time())
            {
              breaks[BREAK_ID_REST_BREAK].override(BREAK_ID_MICRO_BREAK);

              start_break(BREAK_ID_REST_BREAK, BREAK_ID_MICRO_BREAK);

              // Snooze timer before the limit was reached. Just to make sure
              // that it doesn't reach its limit again when elapsed == limit
              rb_timer->snooze_timer();
              return;
            }
        }
    }

  // Stop microbreak when a restbreak starts. should not happened.
  // restbreak should be advanced.
  for (int bi = BREAK_ID_MICRO_BREAK; bi < break_id; bi++)
    {
      if (breaks[bi].get_break_control()->get_break_state() == BreakControl::BREAK_ACTIVE)
        {
          breaks[bi].get_break_control()->stop_break();
        }
    }

  // If break 'break_id' ends, and break 'resume_this_break' is still
  // active, resume it...
  resume_break = resume_this_break;

  BreakControl *breaker = breaks[break_id].get_break_control();
  breaker->start_break();
}

//! Sets the freeze state of all breaks.
void
Core::set_freeze_all_breaks(bool freeze)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      assert(t != nullptr);
      if (!t->has_activity_monitor())
        {
          t->freeze_timer(freeze);
        }
    }
}

void
Core::set_insensitive_mode_all_breaks(InsensitiveMode mode)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      assert(t != nullptr);
      t->set_insensitive_mode(mode);
    }
}

//! Stops all breaks.
void
Core::stop_all_breaks()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakControl *bc = breaks[i].get_break_control();
      assert(bc != nullptr);
      if (bc->is_active())
        {
          bc->stop_break();
        }
    }
}

/********************************************************************************/
/**** Misc                                                                 ******/
/********************************************************************************/

//! Performs a reset when the daily limit is reached.
void
Core::daily_reset()
{
  TRACE_ENTRY();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Timer *t = breaks[i].get_timer();
      assert(t != nullptr);

      int64_t overdue = t->get_total_overdue_time();

      statistics->set_break_counter(((BreakId)i), Statistics::STATS_BREAKVALUE_TOTAL_OVERDUE, (int)overdue);

      t->daily_reset_timer();
    }

  if ((CoreConfig::operation_mode_auto_reset_duration()() == -1min) && (CoreConfig::operation_mode()() != OperationMode::Normal))
    {
      using namespace std::chrono_literals;

      spdlog::debug("Resetting operation mode");
      set_operation_mode(OperationMode::Normal);
      CoreConfig::operation_mode_auto_reset_duration().set(0min);
      CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
    }

  save_state();
}

//! Saves the current state.
void
Core::save_state() const
{
  std::filesystem::path path = Paths::get_state_directory() / "state";
  ofstream stateFile(path.string());

  int64_t current_time = TimeSource::get_real_time_sec();
  stateFile << "WorkRaveState 3" << endl << current_time << endl;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      string stateStr = breaks[i].get_timer()->serialize_state();

      stateFile << stateStr << endl;
    }

  stateFile.close();
}

//! Loads miscellaneous
void
Core::load_misc()
{
  configurator->add_listener(CoreConfig::CFG_KEY_OPERATION_MODE, this);
  configurator->add_listener(CoreConfig::CFG_KEY_USAGE_MODE, this);

  int mode;
  if (!configurator->get_value(CoreConfig::CFG_KEY_OPERATION_MODE, mode))
    {
      mode = underlying_cast(OperationMode::Normal);
    }
  set_operation_mode_internal(OperationMode(mode));
  check_operation_mode_auto_reset();

  if (!configurator->get_value(CoreConfig::CFG_KEY_USAGE_MODE, mode))
    {
      mode = underlying_cast(UsageMode::Normal);
    }
  set_usage_mode(UsageMode(mode));
}

//! Loads the current state.
void
Core::load_state()
{
  std::filesystem::path path = Paths::get_state_directory() / "state";

#if defined(HAVE_TESTS)
  if (hooks->hook_load_timer_state())
    {
      Timer *timers[workrave::BREAK_ID_SIZEOF];
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          timers[i] = breaks[i].get_timer();
        }
      if (hooks->hook_load_timer_state()(timers))
        {
          return;
        }
    }
#endif

  ifstream stateFile(path.string());

  int version = 0;
  bool ok = stateFile.good();

  if (ok)
    {
      string tag;
      stateFile >> tag;

      ok = (tag == WORKRAVESTATE);
    }

  if (ok)
    {
      stateFile >> version;

      ok = (version >= 1 && version <= 3);
    }

  if (ok)
    {
      int64_t saveTime;
      stateFile >> saveTime;
    }

  while (ok && !stateFile.eof())
    {
      string id;
      stateFile >> id;

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          if (breaks[i].get_timer()->get_id() == id)
            {
              string state;
              getline(stateFile, state);

              breaks[i].get_timer()->deserialize_state(state, version);
              break;
            }
        }
    }
}

//! Post an event.
void
Core::post_event(CoreEvent event)
{
  if (core_event_listener != nullptr)
    {
      core_event_listener->core_event_notify(event);
    }
}

//! Execute the insist policy.
void
Core::freeze()
{
  TRACE_ENTRY_PAR(insist_policy);
  InsistPolicy policy = insist_policy;

  switch (policy)
    {
    case InsistPolicy::Ignore:
      {
        // Ignore all activity during break by suspending the activity monitor.
        monitor->suspend();
      }
      break;
    case InsistPolicy::Halt:
      {
        // Halt timer when the user is active.
        set_freeze_all_breaks(true);
      }
      break;
    case InsistPolicy::Reset:
      // reset the timer when the user becomes active.
      // default.
      break;

    default:
      break;
    }

  active_insist_policy = policy;
}

//! Undo the insist policy.
void
Core::defrost()
{
  TRACE_ENTRY_PAR(active_insist_policy);

  switch (active_insist_policy)
    {
    case InsistPolicy::Ignore:
      {
        // Resumes the activity monitor, if not suspended.
        if (operation_mode_active != OperationMode::Suspended)
          {
            monitor->resume();
          }
      }
      break;
    case InsistPolicy::Halt:
      {
        // Desfrost timers.
        set_freeze_all_breaks(false);
      }
      break;

    default:
      break;
    }

  active_insist_policy = InsistPolicy::Invalid;
}

//! Is the user currently active?
bool
Core::is_user_active() const
{
  return monitor_state == ACTIVITY_ACTIVE;
}

namespace workrave
{
  std::string operator%(const string &key, BreakId id)
  {
    IBreak *b = Core::get_instance()->get_break(id);

    string str = key;
    string::size_type pos = 0;
    string name = b->get_name();

    while ((pos = str.find("%b", pos)) != string::npos)
      {
        str.replace(pos, 2, name);
        pos++;
      }

    return str;
  }
} // namespace workrave

boost::signals2::signal<void(OperationMode)> &
Core::signal_operation_mode_changed()
{
  return operation_mode_changed_signal;
}

boost::signals2::signal<void(UsageMode)> &
Core::signal_usage_mode_changed()
{
  return usage_mode_changed_signal;
}
