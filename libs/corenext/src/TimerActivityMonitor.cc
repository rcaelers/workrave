// Copyright (C) 2001 - 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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
#  include <utility>

#  include "config.h"
#endif

#include "TimerActivityMonitor.hh"

#include "debug.hh"

TimerActivityMonitor::TimerActivityMonitor(IActivityMonitor::Ptr monitor, Timer::Ptr timer)
  : monitor(std::move(monitor))
  , timer(std::move(timer))
  , suspended(false)
  , forced_idle(false)
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

bool
TimerActivityMonitor::is_active()
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

  bool running = timer->is_running();
  int64_t idle = timer->get_elapsed_idle_time();
  int64_t reset = timer->get_auto_reset();

  if (!running && idle >= reset)
    {
      TRACE_MSG("Idle stopped");
      return false;
    }

  TRACE_MSG("Active");
  return true;
}

void
TimerActivityMonitor::force_idle()
{
  TRACE_ENTRY();
  TRACE_MSG("Forcing idle");
  forced_idle = true;
}
