//
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

#include "debug.hh"

ReadingActivityMonitor::Ptr
ReadingActivityMonitor::create(IActivityMonitor::Ptr monitor)
{
  return Ptr(new ReadingActivityMonitor(monitor));
}


ReadingActivityMonitor::ReadingActivityMonitor(IActivityMonitor::Ptr monitor) :
  monitor(monitor),
  suspended(false),
  forced_idle(false)
{
}


ReadingActivityMonitor::~ReadingActivityMonitor()
{
}


void
ReadingActivityMonitor::init()
{
  monitor->set_listener(shared_from_this());
}


void
ReadingActivityMonitor::terminate()
{
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


ActivityState
ReadingActivityMonitor::get_state()
{
  TRACE_ENTER("ReadingActivityMonitor::get_current_state");
  if (forced_idle)
    {
      ActivityState local_state = monitor->get_state();
      TRACE_MSG(local_state)

        if (local_state == ACTIVITY_ACTIVE)
          {
            forced_idle = false;
          }
    }

  if (forced_idle)
    {
      TRACE_RETURN("Idle");
      return ACTIVITY_FORCED_IDLE;
    }

  if (suspended)
    {
      TRACE_RETURN("Suspended");
      return ACTIVITY_FORCED_IDLE;
    }

  return state == Active ? ACTIVITY_ACTIVE : ACTIVITY_IDLE;
}


void
ReadingActivityMonitor::force_idle()
{
  TRACE_ENTER("ReadingActivityMonitor::force_idle");
  forced_idle = true;
  TRACE_EXIT();
}


void
ReadingActivityMonitor::set_listener(IActivityMonitorListener::Ptr l)
{
  (void)l;
}


void
ReadingActivityMonitor::handle_break_event(BreakId break_id, IBreak::BreakEvent event)
{
  switch (state)
    {
    case Idle:
      break;

    case Active:
      if (event == IBreak::BREAK_EVENT_PRELUDE_STARTED)
        {
          state = Prelude;
        }
      else if (event == IBreak::BREAK_EVENT_BREAK_STARTED ||
               event == IBreak::BREAK_EVENT_BREAK_STARTED_FORCED)
        {
          state = Taking;
        }
      break;

    case Prelude:
      if (event == IBreak::BREAK_EVENT_BREAK_STARTED)
        {
          state = Taking;
        }
      else if (event == IBreak::BREAK_EVENT_BREAK_IDLE)
        {
          state = Active;
        }
      break;

    case Taking:
      if (event == IBreak::BREAK_EVENT_BREAK_IDLE)
        {
          if (break_id == BREAK_ID_MICRO_BREAK)
            {
              state = Active;
            }
          else
            {
              state = Idle;
              monitor->set_listener(shared_from_this());
            }
        }
    }
}


//!
bool
ReadingActivityMonitor::action_notify()
{
  TRACE_ENTER("ReadingActivityMonitor::action_notify");

  state = Active;
  
  TRACE_EXIT();
  return false;   // false: kill listener.
}

