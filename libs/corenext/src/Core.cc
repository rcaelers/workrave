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

#include <cstdlib>
#include <filesystem>

#include <spdlog/spdlog.h>

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

#if defined(HAVE_GRPC)
#  include "RpcCoreServer.hh"
#endif
#if defined(HAVE_CORE_NEXT_DBUS)
#  include "RpcDBusServer.hh"
#endif

using namespace std;
using namespace workrave;
using namespace workrave::config;
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
  breaks_control = std::make_shared<BreaksControl>(application,
                                                  monitor,
                                                  core_modes,
                                                  statistics,
                                                  hooks);
  breaks_control->init();

#if defined(HAVE_TESTS)
  // Tests install hook_create_monitor() (see above) and construct a fresh
  // Core per test case — starting a real gRPC server bound to a fixed port
  // on every one of those would fight itself for the port and bind a real
  // network listener during CI for no reason. Skip it whenever running under
  // that harness; production Core::init() callers never install these hooks.
  if (!hooks->hook_create_monitor())
#endif
    {
#if defined(HAVE_GRPC)
      init_rpc();
#endif
#if defined(HAVE_CORE_NEXT_DBUS)
      init_rpc_dbus();
#endif
    }
}

#if defined(HAVE_GRPC)
//! Starts the gRPC server exposing CoreService/BreakService/ConfigService.
void
Core::init_rpc()
{
  try
    {
      // A unix domain socket in the per-user state directory, not TCP
      // loopback: no port number to fix, guess, or clash with another
      // instance/process, and access is naturally scoped by filesystem
      // permissions on that directory instead of by whoever can reach
      // 127.0.0.1. Override via WORKRAVE_RPC_ADDRESS for development/testing
      // (e.g. "127.0.0.1:0" for an ephemeral TCP port, printed via the "RPC
      // server listening on" log line) or to point at a different socket.
      std::string listen_address = "unix:" + (Paths::get_state_directory() / "rpc.sock").string();
      if (const char *override_address = std::getenv("WORKRAVE_RPC_ADDRESS"); override_address != nullptr)
        {
          listen_address = override_address;
        }
      rpc_server = std::make_unique<RpcCoreServer>(*this, *configurator, listen_address);
    }
  catch (std::exception &e)
    {
      spdlog::warn("RPC server failed to start: {}", e.what());
    }
}
#endif

#if defined(HAVE_CORE_NEXT_DBUS)
void
Core::init_rpc_dbus()
{
  try
    {
      rpc_dbus_server = std::make_unique<RpcDBusServer>(*this, *configurator);
      spdlog::info("Clang-generated DBus bindings initialized");
    }
  catch (const std::exception &e)
    {
      spdlog::warn("RPC DBus server failed to start: {}", e.what());
    }
}
#endif

#if defined(HAVE_GRPC)
rpc::InstanceRegistry<workrave::BreakId, Break> &
Core::get_break_registry()
{
  return breaks_control->get_break_registry();
}
#endif

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
