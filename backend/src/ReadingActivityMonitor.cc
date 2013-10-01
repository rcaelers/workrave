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
#include "config.h"
#endif

#include "ReadingActivityMonitor.hh"
#include "CoreModes.hh"

#include "debug.hh"

ReadingActivityMonitor::Ptr
ReadingActivityMonitor::create(ActivityMonitor::Ptr monitor, CoreModes::Ptr modes)
{
  return Ptr(new ReadingActivityMonitor(monitor, modes));
}

ReadingActivityMonitor::ReadingActivityMonitor(ActivityMonitor::Ptr monitor, CoreModes::Ptr modes) :
  monitor(monitor),
  modes(modes),
  suspended(false),
  forced_idle(false),
  state(Idle)
{
}

ReadingActivityMonitor::~ReadingActivityMonitor()
{
}

void
ReadingActivityMonitor::init()
{
  monitor->set_listener(shared_from_this());

  modes->signal_usage_mode_changed().connect(boost::bind(&ReadingActivityMonitor::on_usage_mode_changed, this, _1)); 
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
  TRACE_ENTER("ReadingActivityMonitor::is_active");
  if (forced_idle)
    {
      bool local_is_active = monitor->is_active();
      TRACE_MSG(local_is_active)

        if (local_is_active)
          {
            forced_idle = false;
          }
    }

  if (forced_idle)
    {
      TRACE_RETURN("Idle");
      return false;
    }

  if (suspended)
    {
      TRACE_RETURN("Suspended");
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
  
  TRACE_RETURN(active);
  return active;
}

void
ReadingActivityMonitor::force_idle()
{
  TRACE_ENTER("ReadingActivityMonitor::force_idle");
  forced_idle = true;
  TRACE_EXIT();
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
ReadingActivityMonitor::handle_break_event(BreakId break_id, BreakEvent event)
{
  TRACE_ENTER_MSG("ReadingActivityMonitor::handle_break_event", break_id << " " << static_cast<std::underlying_type<BreakEvent>::type>(event));
  switch (state)
    {
    case Idle:
      break;

    case Active:
      if (event == BreakEvent::ShowPrelude)
        {
          TRACE_MSG("Active -> Prelude");
          state = Prelude;
        }
      else if (event == BreakEvent::ShowBreak ||
               event == BreakEvent::ShowBreakForced)
        {
          TRACE_MSG("Active -> Taking");
          state = Taking;
        }
      break;

    case Prelude:
      if (event == BreakEvent::ShowBreak ||
          event == BreakEvent::ShowBreakForced)
        {
          TRACE_MSG("Prelude -> Taking");
          state = Taking;
        }
      else if (event == BreakEvent::BreakIdle)
        {
          TRACE_MSG("Prelude -> Active");
          state = Active;
          forced_idle = false;
        }
      break;

    case Taking:
      if (event == BreakEvent::BreakIdle)
        {
          if (break_id == BREAK_ID_MICRO_BREAK)
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
  TRACE_EXIT();
}

bool
ReadingActivityMonitor::action_notify()
{
  state = Active;
  return false;   // false: kill listener.
}
