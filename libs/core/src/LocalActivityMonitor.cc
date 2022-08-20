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
#  include "config.h"
#endif

#include "LocalActivityMonitor.hh"
#include "IActivityMonitorListener.hh"

#include "debug.hh"

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/InputMonitorFactory.hh"

using namespace std;
using namespace workrave::utils;

//! Constructor.
LocalActivityMonitor::LocalActivityMonitor()
{
  TRACE_ENTRY();
  noise_threshold = 1 * workrave::utils::TimeSource::TIME_USEC_PER_SEC;
  activity_threshold = 2 * workrave::utils::TimeSource::TIME_USEC_PER_SEC;
  idle_threshold = 5 * workrave::utils::TimeSource::TIME_USEC_PER_SEC;

  input_monitor = workrave::input_monitor::InputMonitorFactory::create_monitor(
    workrave::input_monitor::MonitorCapability::Activity);
  if (input_monitor != nullptr)
    {
      input_monitor->subscribe(this);
    }
}

//! Destructor.
LocalActivityMonitor::~LocalActivityMonitor()
{
  TRACE_ENTRY();
  if (input_monitor != NULL)
    {
      input_monitor->unsubscribe(this);
    }
}

//! Terminates the monitor.
void
LocalActivityMonitor::terminate()
{
  TRACE_ENTRY();
  if (input_monitor != nullptr)
    {
      input_monitor->terminate();
    }
}

//! Suspends the activity monitoring.
void
LocalActivityMonitor::suspend()
{
  TRACE_ENTRY_PAR(activity_state);
  lock.lock();
  activity_state = ACTIVITY_SUSPENDED;
  lock.unlock();
  activity_state.publish();
  TRACE_VAR(activity_state);
}

//! Resumes the activity monitoring.
void
LocalActivityMonitor::resume()
{
  TRACE_ENTRY_PAR(activity_state);
  lock.lock();
  activity_state = ACTIVITY_IDLE;
  lock.unlock();
  activity_state.publish();
  TRACE_VAR(activity_state);
}

//! Forces state te be idle.
void
LocalActivityMonitor::force_idle()
{
  TRACE_ENTRY_PAR(activity_state);
  lock.lock();
  if (activity_state != ACTIVITY_SUSPENDED)
    {
      activity_state = ACTIVITY_IDLE;
      last_action_time = 0;
    }
  lock.unlock();
  activity_state.publish();
  TRACE_VAR(activity_state);
}

//! Returns the current state
ActivityState
LocalActivityMonitor::get_current_state()
{
  TRACE_ENTRY_PAR(activity_state);
  lock.lock();

  // First update the state...
  if (activity_state == ACTIVITY_ACTIVE)
    {
      int64_t tv = TimeSource::get_monotonic_time_usec() - last_action_time;
      if (tv > idle_threshold)
        {
          // No longer active.
          activity_state = ACTIVITY_IDLE;
        }
    }

  lock.unlock();
  activity_state.publish();
  TRACE_VAR(activity_state);
  return activity_state;
}

//! Sets the operation parameters.
void
LocalActivityMonitor::set_parameters(int noise, int activity, int idle, int sensitivity)
{
  noise_threshold = noise * 1000;
  activity_threshold = activity * 1000;
  idle_threshold = idle * 1000;

  this->sensitivity = sensitivity;

  // The easy way out.
  activity_state = ACTIVITY_IDLE;
}

//! Sets the operation parameters.
void
LocalActivityMonitor::get_parameters(int &noise, int &activity, int &idle, int &sensitivity)
{
  noise = noise_threshold / 1000;
  activity = activity_threshold / 1000;
  idle = idle_threshold / 1000;
  sensitivity = this->sensitivity;
}

//! Shifts the internal time (after system clock has been set)
void
LocalActivityMonitor::shift_time(int delta)
{
  int64_t d = delta * workrave::utils::TimeSource::TIME_USEC_PER_SEC;

  Diagnostics::instance().log("activity_monitor: shift");
  lock.lock();

  if (last_action_time != 0)
    last_action_time += d;

  if (first_action_time != 0)
    first_action_time += d;

  lock.unlock();
}

//! Sets the callback listener.
void
LocalActivityMonitor::set_listener(IActivityMonitorListener *l)
{
  lock.lock();
  listener = l;
  lock.unlock();
}

//! Activity is reported by the input monitor.
void
LocalActivityMonitor::action_notify()
{
  lock.lock();

  int64_t now = TimeSource::get_monotonic_time_usec();

  switch (activity_state)
    {
    case ACTIVITY_IDLE:
      {
        first_action_time = now;
        last_action_time = now;

        if (activity_threshold == 0)
          {
            activity_state = ACTIVITY_ACTIVE;
          }
        else
          {
            activity_state = ACTIVITY_NOISE;
          }
      }
      break;

    case ACTIVITY_NOISE:
      {
        int64_t tv = now - last_action_time;
        if (tv > noise_threshold)
          {
            first_action_time = now;
          }
        else
          {
            tv = now - first_action_time;
            if (tv >= activity_threshold)
              {
                activity_state = ACTIVITY_ACTIVE;
              }
          }
      }
      break;

    default:
      break;
    }

  last_action_time = now;
  lock.unlock();
  call_listener();
}

//! Mouse activity is reported by the input monitor.
void
LocalActivityMonitor::mouse_notify(int x, int y, int wheel_delta)
{
  lock.lock();
  const int delta_x = x - prev_x;
  const int delta_y = y - prev_y;
  prev_x = x;
  prev_y = y;

  if (abs(delta_x) >= sensitivity || abs(delta_y) >= sensitivity || wheel_delta != 0 || button_is_pressed)
    {
      action_notify();
    }
  lock.unlock();
}

//! Mouse button activity is reported by the input monitor.
void
LocalActivityMonitor::button_notify(bool is_press)
{
  lock.lock();

  button_is_pressed = is_press;

  if (is_press)
    {
      action_notify();
    }

  lock.unlock();
}

//! Keyboard activity is reported by the input monitor.
void
LocalActivityMonitor::keyboard_notify(bool repeat)
{
  (void)repeat;

  lock.lock();
  action_notify();
  lock.unlock();
}

//! Calls the callback listener.
void
LocalActivityMonitor::call_listener()
{
  IActivityMonitorListener *l = nullptr;

  lock.lock();
  l = listener;
  lock.unlock();

  if (l != nullptr)
    {
      // Listener is set.
      if (!l->action_notify())
        {
          // Remove listener.
          lock.lock();
          listener = nullptr;
          lock.unlock();
        }
    }
}
