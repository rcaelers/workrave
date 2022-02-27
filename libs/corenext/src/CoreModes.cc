// Copyright (C) 2001 - 2013 Ray Satiro & Rob Caelers & Raymond Penners
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

#include <chrono>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "debug.hh"

#include "CoreModes.hh"
#include "core/CoreTypes.hh"
#include "core/CoreConfig.hh"
#include "utils/TimeSource.hh"

using namespace std;
using namespace workrave;

CoreModes::CoreModes(IActivityMonitor::Ptr monitor)
  : operation_mode_active(OperationMode::Normal)
  , operation_mode_regular(OperationMode::Normal)
  , usage_mode(UsageMode::Normal)
  , monitor(monitor)
{
  TRACE_ENTRY();
  load_config();
}

CoreModes::~CoreModes()
{
  TRACE_ENTRY();
}

boost::signals2::signal<void(OperationMode)> &
CoreModes::signal_operation_mode_changed()
{
  return operation_mode_changed_signal;
}

boost::signals2::signal<void(UsageMode)> &
CoreModes::signal_usage_mode_changed()
{
  return usage_mode_changed_signal;
}

//! Retrieves the operation mode.
OperationMode
CoreModes::get_active_operation_mode()
{
  return operation_mode_active;
}

//! Retrieves the regular operation mode.
OperationMode
CoreModes::get_regular_operation_mode()
{
  /* operation_mode_regular is the same as operation_mode unless there's an
     override in place, in which case operation_mode is the current override mode and
     operation_mode_regular is the mode that will be restored once all overrides are removed
  */
  return operation_mode_regular;
}

//! Checks if operation_mode is an override.
bool
CoreModes::is_operation_mode_an_override()
{
  return !operation_mode_overrides.empty();
}

//! Sets the operation mode.
void
CoreModes::set_operation_mode(OperationMode mode)
{
  using namespace std::chrono_literals;

  set_operation_mode_internal(mode);
  CoreConfig::operation_mode_auto_reset_duration().set(0min);
  CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
}

void
CoreModes::set_operation_mode_for(OperationMode mode, std::chrono::minutes duration)
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
CoreModes::set_operation_mode_override(OperationMode mode, const std::string &id)
{
  if (!id.empty())
    {
      operation_mode_overrides[id] = mode;
      update_active_operation_mode();
    }
}

//! Removes the overridden operation mode.
void
CoreModes::remove_operation_mode_override(const std::string &id)
{
  if (!id.empty() && !operation_mode_overrides.empty())
    {
      operation_mode_overrides.erase(id);
      update_active_operation_mode();
    }
}

//! Set the operation mode.
void
CoreModes::set_operation_mode_internal(OperationMode mode)
{
  if (operation_mode_regular != mode)
    {
      operation_mode_regular = mode;
      update_active_operation_mode();
      CoreConfig::operation_mode().set(mode);
      operation_mode_changed_signal(operation_mode_regular);
    }
}

void
CoreModes::update_active_operation_mode()
{
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

  if (operation_mode_active != mode)
    {
      OperationMode previous_mode = operation_mode_active;
      operation_mode_active = mode;

      if (operation_mode_active == OperationMode::Suspended)
        {
          monitor->suspend();
        }
      else if (previous_mode == OperationMode::Suspended)
        {
          monitor->resume();
        }
    }
}

//! Retrieves the usage mode.
UsageMode
CoreModes::get_usage_mode()
{
  return usage_mode;
}

//! Sets the usage mode.
void
CoreModes::set_usage_mode(UsageMode mode)
{
  set_usage_mode_internal(mode, true);
}

//! Sets the usage mode.
void
CoreModes::set_usage_mode_internal(UsageMode mode, bool persistent)
{
  if (usage_mode != mode)
    {
      usage_mode = mode;

      if (persistent)
        {
          CoreConfig::usage_mode().set(mode);
        }

      usage_mode_changed_signal(mode);
    }
}

void
CoreModes::load_config()
{
  TRACE_ENTRY();
  CoreConfig::operation_mode().connect(this, [this](OperationMode operation_mode) {
    spdlog::debug("Operation mode changed {} {} {}", operation_mode_regular, operation_mode_active, operation_mode);
    set_operation_mode_internal(operation_mode);
  });

  CoreConfig::usage_mode().connect(this, [this](UsageMode usage_mode) { set_usage_mode_internal(usage_mode, false); });
  OperationMode operation_mode = CoreConfig::operation_mode()();
  TRACE_VAR(operation_mode);
  set_operation_mode_internal(operation_mode);

  check_auto_reset();

  UsageMode usage_mode = CoreConfig::usage_mode()();
  TRACE_VAR(usage_mode);
  set_usage_mode(usage_mode);
}

void
CoreModes::check_auto_reset()
{
  auto next_reset_time = CoreConfig::operation_mode_auto_reset_time()();

  if ((next_reset_time.time_since_epoch().count() > 0) && (workrave::utils::TimeSource::get_real_time() >= next_reset_time)
      && (CoreConfig::operation_mode()() != OperationMode::Normal))
    {
      spdlog::debug("Resetting operation mode");
      set_operation_mode(OperationMode::Normal);
      CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
    }
}

void
CoreModes::heartbeat()
{
  check_auto_reset();
}

//! Performs a reset when the daily limit is reached.
void
CoreModes::daily_reset()
{
  if ((CoreConfig::operation_mode_auto_reset_duration()() == -1min) && (CoreConfig::operation_mode()() != OperationMode::Normal))
    {
      using namespace std::chrono_literals;

      spdlog::debug("Resetting operation mode");
      set_operation_mode(OperationMode::Normal);
      CoreConfig::operation_mode_auto_reset_duration().set(0min);
      CoreConfig::operation_mode_auto_reset_time().set(std::chrono::system_clock::time_point{});
    }
}
