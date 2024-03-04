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

#include <filesystem>

#include "Core.hh"

#include "config/ConfiguratorFactory.hh"
#include "config/IConfigurator.hh"
#include "utils/TimeSource.hh"
#include "input-monitor/InputMonitorFactory.hh"

#include "utils/Paths.hh"
#include "utils/AssetPath.hh"
#include "core/IApp.hh"
#include "Break.hh"
#include "core/CoreConfig.hh"
#include "Statistics.hh"

#include "dbus/DBusFactory.hh"
#if defined(HAVE_DBUS)
#  include "DBusWorkraveNext.hh"
#  define DBUS_PATH_WORKRAVE "/org/workrave/Workrave/"
#  define DBUS_SERVICE_WORKRAVE "org.workrave.Workrave"
#endif

using namespace std;
using namespace workrave;
using namespace workrave::config;
using namespace workrave::dbus;
using namespace workrave::utils;

ICore::Ptr
CoreFactory::create(workrave::config::IConfigurator::Ptr configurator)
{
  return std::make_shared<Core>(configurator);
}

Core::Core(workrave::config::IConfigurator::Ptr configurator)
  : configurator(configurator)
{
  TRACE_ENTRY();
  hooks = std::make_shared<CoreHooks>();
  TimeSource::sync();
}

Core::~Core()
{
  TRACE_ENTRY();
  if (monitor)
    {
      monitor->terminate();
    }
}

void
Core::init(IApp *app, const char *display_name)
{
  application = app;

  CoreConfig::init(configurator);

  dbus = DBusFactory::create();
  dbus->init();

#if defined(HAVE_TESTS)
  if (hooks->hook_create_monitor())
    {
      monitor = hooks->hook_create_monitor()();
    }
  else
#endif
    {
      // LCOV_EXCL_START
      monitor = std::make_shared<LocalActivityMonitor>(configurator, display_name);
      // LCOV_EXCL_STOP
    }

  monitor->init();

  statistics = std::make_shared<Statistics>(monitor);
  statistics->init();

  core_modes = std::make_shared<CoreModes>(monitor);
  core_dbus = std::make_shared<CoreDBus>(core_modes, dbus);

  breaks_control = std::make_shared<BreaksControl>(application, monitor, core_modes, statistics, dbus, hooks);
  breaks_control->init();

  init_bus();
}

//! Initializes the communication bus.
void
Core::init_bus()
{
#if defined(HAVE_DBUS)
  try
    {
      extern void init_DBusWorkraveNext(IDBus::Ptr dbus);
      init_DBusWorkraveNext(dbus);

      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.CoreInterface", this);
      dbus->connect(DBUS_PATH_WORKRAVE "Core", "org.workrave.ConfigInterface", configurator.get());
      dbus->register_object_path(DBUS_PATH_WORKRAVE "Core");
    }
  catch (DBusException &)
    {
    }
#endif
}

//! Periodic heartbeat.
void
Core::heartbeat()
{
  TRACE_ENTRY();
  TimeSource::sync();

  configurator->heartbeat();
  breaks_control->heartbeat();
  core_modes->heartbeat();
}

/********************************************************************************/
/**** ICore Interface                                                      ******/
/********************************************************************************/

boost::signals2::signal<void(OperationMode)> &
Core::signal_operation_mode_changed()
{
  return core_modes->signal_operation_mode_changed();
}

boost::signals2::signal<void(UsageMode)> &
Core::signal_usage_mode_changed()
{
  return core_modes->signal_usage_mode_changed();
}

//! Forces the start of the specified break.
void
Core::force_break(BreakId id, workrave::utils::Flags<BreakHint> break_hint)
{
  breaks_control->force_break(id, break_hint);
}

//!
bool
Core::is_taking() const
{
  bool taking = false;
  for (BreakId break_id = BREAK_ID_MICRO_BREAK; break_id < BREAK_ID_SIZEOF; break_id++)
    {
      if (get_break(break_id)->is_taking())
        {
          taking = true;
        }
    }

  return taking;
}

//! Returns the specified break controller.
IBreak::Ptr
Core::get_break(BreakId id) const
{
  return breaks_control->get_break(id);
}

//! Returns the statistics.
IStatistics::Ptr
Core::get_statistics() const
{
  return statistics;
}

//!
ICoreHooks::Ptr
Core::get_hooks() const
{
  return hooks;
}

dbus::IDBus::Ptr
Core::get_dbus() const
{
  return dbus;
}

//! Is the user currently active?
bool
Core::is_user_active() const
{
  return monitor->is_active();
}

//! Retrieves the operation mode.
OperationMode
Core::get_active_operation_mode()
{
  return core_modes->get_active_operation_mode();
}

//! Retrieves the regular operation mode.
OperationMode
Core::get_regular_operation_mode()
{
  return core_modes->get_regular_operation_mode();
}

//! Checks if operation_mode is an override.
bool
Core::is_operation_mode_an_override()
{
  return core_modes->is_operation_mode_an_override();
}

//! Sets the operation mode.
void
Core::set_operation_mode(OperationMode mode)
{
  core_modes->set_operation_mode(mode);
}

void
Core::set_operation_mode_for(OperationMode mode, std::chrono::minutes duration)
{
  core_modes->set_operation_mode_for(mode, duration);
}

//! Temporarily overrides the operation mode.
void
Core::set_operation_mode_override(OperationMode mode, const std::string &id)
{
  core_modes->set_operation_mode_override(mode, id);
}

//! Removes the overridden operation mode.
void
Core::remove_operation_mode_override(const std::string &id)
{
  core_modes->remove_operation_mode_override(id);
}

//! Retrieves the usage mode.
UsageMode
Core::get_usage_mode()
{
  return core_modes->get_usage_mode();
}

//! Sets the usage mode.
void
Core::set_usage_mode(UsageMode mode)
{
  core_modes->set_usage_mode(mode);
}

//! Sets the insist policy.
/*!
 *  The insist policy determines what to do when the user is active while
 *  taking a break.
 */
void
Core::set_insist_policy(InsistPolicy p)
{
  breaks_control->set_insist_policy(p);
}

//! Forces all monitors to be idle.
void
Core::force_idle()
{
  monitor->force_idle();
}

//! Announces a powersave state.
void
Core::set_powersave(bool down)
{
  TRACE_ENTRY_PAR(down);
  TRACE_VAR(powersave, core_modes->get_active_operation_mode());

  if (down)
    {
      if (!powersave)
        {
          // Computer is going down
          set_operation_mode_override(OperationMode::Suspended, "powersave");
          powersave = true;
        }

      breaks_control->save_state();
      statistics->update();
    }
  else
    {
      remove_operation_mode_override("powersave");
      powersave = false;
    }
}

void
Core::report_external_activity(std::string who, bool act)
{
  (void)who;
  (void)act;
  // TODO: fix this
  // monitor->report_external_activity(who, act);
}

// TODO: remove
namespace workrave
{
  std::string operator%(const string &key, BreakId id)
  {
    string str = key;
    string::size_type pos = 0;
    string name = CoreConfig::get_break_name(id);

    while ((pos = str.find("%b", pos)) != string::npos)
      {
        str.replace(pos, 2, name);
        pos++;
      }

    return str;
  }
} // namespace workrave
