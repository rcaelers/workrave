// TimerActivityMonitor.hh
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef TIMERACTIVITYMONITOR_HH
#define TIMERACTIVITYMONITOR_HH

#include "Core.hh"
#include "IActivityMonitor.hh"

//! An Activity Monitor that takes its activity state from a timer.
/*! This Activity Monitor is 'active' if the timer is running or if
 *  the timer has its idle time not at maximum, 'idle' otherwise.
 */
class TimerActivityMonitor : public IActivityMonitor
{
public:
  //! Constructs an activity monitor that depends on specified timer.
  TimerActivityMonitor(Timer *t) :
    timer(t),
    suspended(false),
    forced_idle(false)

  {
    Core *core =  Core::get_instance();
    monitor = core->get_activity_monitor();
  }

  virtual ~TimerActivityMonitor()
  {
  }

  //! Stops the activity monitoring.
  void terminate()
  {
  }

  //! Suspends the activity monitoring.
  void suspend()
  {
    suspended = true;
  }

  //! Resumes the activity monitoring.
  void resume()
  {
    suspended = false;
  }

  //! Returns the current state
  ActivityState get_current_state()
  {
    TRACE_ENTER("TimerActivityMonitor::get_current_state");
    if (forced_idle)
      {
        ActivityState local_state = monitor->get_current_state();
        TRACE_MSG(local_state)
          
        if (local_state != ACTIVITY_IDLE &&
            local_state != ACTIVITY_SUSPENDED)
          {
            forced_idle = false;
          }

      }

    if (forced_idle)
      {
        TRACE_RETURN("Idle");
        return ACTIVITY_IDLE;
      }

    if (suspended)
      {
        TRACE_RETURN("Suspended");
        return ACTIVITY_SUSPENDED;
      }

    TimerState state = timer->get_state();
    time_t idle = timer->get_elapsed_idle_time();
    time_t reset = timer->get_auto_reset();

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

  //! Force state to be idle.
  void force_idle()
  {
    TRACE_ENTER("TimerActivityMonitor::force_idle");
    // TRACE_MSG("Forcing idle");
    // forced_idle = true;
    TRACE_EXIT();
  }


  // Returns the collected statistics.
  //! Sets the activity listener of this monitor.
  void set_listener(ActivityMonitorListener *l)
  {
    (void)l;
  }

private:
  //! Reference monitor
  IActivityMonitor *monitor;

  //! Reference timer.
  Timer *timer;

  //! Monitor suspended?
  bool suspended;

  //! Is this timer forced idle?
  bool forced_idle;
};

#endif // TIMERACTIVITYMONITOR_HH
