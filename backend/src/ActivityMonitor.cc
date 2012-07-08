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
ActivityMonitor::create(IConfigurator::Ptr configurator)
{
  return Ptr(new ActivityMonitor(configurator));
}

    
//! Constructor.
ActivityMonitor::ActivityMonitor(IConfigurator::Ptr configurator) :
  configurator(configurator)
{
  TRACE_ENTER("ActivityMonitor::ActivityMonitor");
  local_monitor = LocalActivityMonitor::create(configurator);
  TRACE_EXIT();
}


//! Destructor.
ActivityMonitor::~ActivityMonitor()
{
  TRACE_ENTER("ActivityMonitor::~ActivityMonitor");
  TRACE_EXIT();
}


//! Initializes the monitor.
void
ActivityMonitor::init(const string &display_name)
{
  TRACE_ENTER("ActivityMonitor::init");
  local_monitor->init(display_name);
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
ActivityMonitor::get_current_state()
{
  return ACTIVITY_IDLE;
}


//! Shifts the internal time (after system clock has been set)
void
ActivityMonitor::shift_time(int delta)
{
}


//! Sets the callback listener.
void
ActivityMonitor::set_listener(IActivityMonitorListener::Ptr l)
{
}

//! Computes the current state.
void
ActivityMonitor::heartbeat()
{
  // Default
  ActivityState local_state = local_monitor->get_current_state();
  gint64 current_time = TimeSource::get_monotonic_time();
  
  map<std::string, gint64>::iterator i = external_activity.begin();
  while (i != external_activity.end())
    {
      map<std::string, gint64>::iterator next = i;
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
ActivityMonitor::report_external_activity(std::string who, bool act)
{
  TRACE_ENTER_MSG("ActivityMonitor::report_external_activity", who << " " << act);
  if (act)
    {
      external_activity[who] = TimeSource::get_monotonic_time() + 10;
    }
  else
    {
      external_activity.erase(who);
    }
  TRACE_EXIT();
}
