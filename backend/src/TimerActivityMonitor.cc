//
// Copyright (C) 2001 - 2010, 2012 Rob Caelers <robc@krandor.nl>
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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

#include "TimerActivityMonitor.hh"

#include "debug.hh"

TimerActivityMonitor::Ptr
TimerActivityMonitor::create(IActivityMonitor::Ptr monitor, Timer::Ptr timer)
{
  return Ptr(new TimerActivityMonitor(monitor, timer));
}

TimerActivityMonitor::TimerActivityMonitor(IActivityMonitor::Ptr monitor, Timer::Ptr timer) :
  monitor(monitor),
  timer(timer),
  suspended(false),
  forced_idle(false)
{
}


TimerActivityMonitor::~TimerActivityMonitor()
{
}


void
TimerActivityMonitor::terminate()
{
}


void
TimerActivityMonitor::suspend()
{
  suspended = true;
}


void
TimerActivityMonitor::resume()
{
  suspended = false;
}


ActivityState
TimerActivityMonitor::get_current_state()
{
  TRACE_ENTER("TimerActivityMonitor::get_current_state");
  if (forced_idle)
    {
      ActivityState local_state = monitor->get_current_state();
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

  TimerState state = timer->get_state();
  gint64 idle = timer->get_elapsed_idle_time();
  gint64 reset = timer->get_auto_reset();

  if (state == STATE_STOPPED && idle >= reset)
    {
      TRACE_RETURN("Idle stopped");
      return ACTIVITY_IDLE;
    }
  else
    {
      TRACE_RETURN("Active");
      return ACTIVITY_ACTIVE;
    }
}


void
TimerActivityMonitor::force_idle()
{
  TRACE_ENTER("TimerActivityMonitor::force_idle");
  TRACE_MSG("Forcing idle");
  forced_idle = true;
  TRACE_EXIT();
}


void
TimerActivityMonitor::set_listener(IActivityMonitorListener::Ptr l)
{
  (void)l;
}
