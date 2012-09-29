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

#include "ActivityMonitor.hh"

#include "utils/TimeSource.hh"

#include "IActivityMonitorListener.hh"
#include "CoreConfig.hh"
#include "debug.hh"

using namespace std;
using namespace workrave::utils;

ActivityMonitor::Ptr
ActivityMonitor::create(IConfigurator::Ptr configurator, CoreHooks::Ptr hooks, const std::string &display_name)
{
  return Ptr(new ActivityMonitor(configurator, hooks, display_name));
}

    
//! Constructor.
ActivityMonitor::ActivityMonitor(IConfigurator::Ptr configurator, CoreHooks::Ptr hooks, const std::string &display_name) :
  configurator(configurator), hooks(hooks), monitor_state(ACTIVITY_IDLE)
{
  TRACE_ENTER("ActivityMonitor::ActivityMonitor");
  local_monitor = LocalActivityMonitor::create(configurator, display_name);
  TRACE_EXIT();
}

//! Initializes the monitor.
void
ActivityMonitor::init()
{
  TRACE_ENTER("ActivityMonitor::init");
  local_monitor->init();
  TRACE_EXIT();
}


//! Terminates the monitor.
void
ActivityMonitor::terminate()
{
  TRACE_ENTER("ActivityMonitor::terminate");
  local_monitor->terminate();
  TRACE_EXIT();
}


//! Suspends the activity monitoring.
void
ActivityMonitor::suspend()
{
  TRACE_ENTER("ActivityMonitor::suspend");
  local_monitor->suspend();
  TRACE_EXIT();
}


//! Resumes the activity monitoring.
void
ActivityMonitor::resume()
{
  TRACE_ENTER("ActivityMonitor::resume");
  local_monitor->resume();
  TRACE_EXIT();
}


//! Forces state te be idle.
void
ActivityMonitor::force_idle()
{
  TRACE_ENTER("ActivityMonitor::force_idle");
  local_monitor->force_idle();
  TRACE_EXIT();
}


//! Returns the current state
ActivityState
ActivityMonitor::get_state()
{
  return monitor_state;
}

//! Sets the callback listener.
void
ActivityMonitor::set_listener(IActivityMonitorListener::Ptr l)
{
  local_monitor->set_listener(l);
}


//! Computes the current state.
void
ActivityMonitor::heartbeat()
{
  // Default
  ActivityState state;
  
  if (!hooks->hook_local_activity_state().empty())
    {
      state = hooks->hook_local_activity_state()();
    }
  else
    {
      state = local_monitor->get_state();
    }
  
  gint64 current_time = TimeSource::get_monotonic_time_sec();
  
  map<std::string, gint64>::iterator i = external_activity.begin();
  while (i != external_activity.end())
    {
      map<std::string, gint64>::iterator next = i;
      next++;

      if (i->second >= current_time)
        {
          state = ACTIVITY_ACTIVE;
        }
      else
        {
          external_activity.erase(i);
        }

      i = next;
    }

  if (local_state != state)
    {
      local_state = state;
      hooks->signal_local_active_changed()(local_state == ACTIVITY_ACTIVE);
    }

  monitor_state = state;

  if (!hooks->hook_is_active().empty())
    {
      bool hook_is_active = (hooks->hook_is_active()());
      if (hook_is_active)
        {
          monitor_state = ACTIVITY_ACTIVE;
        }
    }
}


void
ActivityMonitor::report_external_activity(std::string who, bool act)
{
  TRACE_ENTER_MSG("ActivityMonitor::report_external_activity", who << " " << act);
  if (act)
    {
      external_activity[who] = TimeSource::get_monotonic_time_sec() + 10;
    }
  else
    {
      external_activity.erase(who);
    }
  TRACE_EXIT();
}
