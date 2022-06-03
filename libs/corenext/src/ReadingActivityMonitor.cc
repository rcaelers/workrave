// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include "ReadingActivityMonitor.hh"
#include "CoreModes.hh"

#include "debug.hh"

using namespace workrave::input_monitor;
ReadingActivityMonitor::ReadingActivityMonitor(IActivityMonitor::Ptr monitor, CoreModes::Ptr modes)
  : monitor(monitor)
  , modes(modes)
  , suspended(false)
  , forced_idle(false)
  , state(Idle)
{
}

void
ReadingActivityMonitor::init()
{
  monitor->set_listener(shared_from_this());
  connect(modes->signal_usage_mode_changed(), this, [this](auto &&mode) {
    on_usage_mode_changed(std::forward<decltype(mode)>(mode));
  });
}

void
ReadingActivityMonitor::suspend()
{
  suspended = true;
}

void
ReadingActivityMonitor::resume()
{
  suspended = false;
}

bool
ReadingActivityMonitor::is_active()
{
  TRACE_ENTRY();
  if (forced_idle)
    {
      bool local_is_active = monitor->is_active();
      TRACE_VAR(local_is_active);

      if (local_is_active)
        {
          forced_idle = false;
        }
    }

  if (forced_idle)
    {
      TRACE_MSG("Idle");
      return false;
    }

  if (suspended)
    {
      TRACE_MSG("Suspended");
      return false;
    }

  bool active = false;
  switch (state)
    {
    case Idle:
      break;

    case Active:
      active = true;
      break;

    case Prelude:
    case Taking:
      active = monitor->is_active();
      break;
    }

  TRACE_VAR(active);
  return active;
}

void
ReadingActivityMonitor::force_idle()
{
  TRACE_ENTRY();
  forced_idle = true;
}

void
ReadingActivityMonitor::on_usage_mode_changed(workrave::UsageMode mode)
{
  if (mode == workrave::UsageMode::Reading)
    {
      state = Idle;
    }
}

void
ReadingActivityMonitor::handle_break_event(workrave::BreakId break_id, workrave::BreakEvent event)
{
  TRACE_ENTRY_PAR(break_id, static_cast<std::underlying_type<workrave::BreakEvent>::type>(event));
  switch (state)
    {
    case Idle:
      break;

    case Active:
      if (event == workrave::BreakEvent::ShowPrelude)
        {
          TRACE_MSG("Active -> Prelude");
          state = Prelude;
        }
      else if (event == workrave::BreakEvent::ShowBreak || event == workrave::BreakEvent::ShowBreakForced)
        {
          TRACE_MSG("Active -> Taking");
          state = Taking;
        }
      break;

    case Prelude:
      if (event == workrave::BreakEvent::ShowBreak || event == workrave::BreakEvent::ShowBreakForced)
        {
          TRACE_MSG("Prelude -> Taking");
          state = Taking;
        }
      else if (event == workrave::BreakEvent::BreakIdle)
        {
          TRACE_MSG("Prelude -> Active");
          state = Active;
          forced_idle = false;
        }
      break;

    case Taking:
      if (event == workrave::BreakEvent::BreakIdle)
        {
          if (break_id == workrave::BREAK_ID_MICRO_BREAK)
            {
              TRACE_MSG("Taking -> Active");
              state = Active;
              forced_idle = false;
            }
          else
            {
              TRACE_MSG("Taking -> Idle");
              state = Idle;
              monitor->set_listener(shared_from_this());
            }
        }
    }
}

bool
ReadingActivityMonitor::action_notify()
{
  state = Active;
  return false; // false: kill listener.
}
