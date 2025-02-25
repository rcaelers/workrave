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

#include <cassert>
#include <cmath>
#include <cstddef>
#include <utility>

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/InputMonitorFactory.hh"

#include "core/CoreConfig.hh"
#include "debug.hh"

using namespace std;
using namespace workrave::config;
using namespace workrave::input_monitor;
using namespace workrave::utils;

LocalActivityMonitor::LocalActivityMonitor(IConfigurator::Ptr config, const char *display_name)
  : config(std::move(config))
  , display_name(display_name)
{
  TRACE_ENTRY();
}

//! Initializes the monitor.
void
LocalActivityMonitor::init()
{
  TRACE_ENTRY();
  InputMonitorFactory::init(config, display_name);

  load_config();
  CoreConfig::key_monitor().connect(this, [this] { load_config(); });

  input_monitor = InputMonitorFactory::create_monitor(MonitorCapability::Activity);
  if (input_monitor != nullptr)
    {
      input_monitor->subscribe(this);
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
  TRACE_ENTRY_PAR(state);
  lock.lock();
  state = ACTIVITY_MONITOR_SUSPENDED;
  lock.unlock();
  TRACE_VAR(state);
}

//! Resumes the activity monitoring.
void
LocalActivityMonitor::resume()
{
  TRACE_ENTRY_PAR(state);
  lock.lock();
  state = ACTIVITY_MONITOR_IDLE;
  lock.unlock();
  TRACE_VAR(state);
}

//! Forces state te be idle.
void
LocalActivityMonitor::force_idle()
{
  TRACE_ENTRY_PAR(state);
  lock.lock();
  if (state != ACTIVITY_MONITOR_SUSPENDED)
    {
      state = ACTIVITY_MONITOR_FORCED_IDLE;
      last_action_time = 0;
    }
  lock.unlock();
  TRACE_VAR(state);
}

bool
LocalActivityMonitor::is_active()
{
  process_state();
  return state == ACTIVITY_MONITOR_ACTIVE;
}

void
LocalActivityMonitor::process_state()
{
  TRACE_ENTRY_PAR(state);
  lock.lock();

  // First update the state...
  if (state == ACTIVITY_MONITOR_ACTIVE)
    {
      int64_t tv = TimeSource::get_monotonic_time_usec() - last_action_time;

      TRACE_MSG("Active: {} {}", tv, idle_threshold);
      if (tv > idle_threshold)
        {
          // No longer active.
          state = ACTIVITY_MONITOR_IDLE;
        }
    }

  lock.unlock();
  TRACE_VAR(state);
}

//! Sets the operation parameters.
void
LocalActivityMonitor::set_parameters(int noise, int activity, int idle, int sensitivity)
{
  noise_threshold = static_cast<int64_t>(noise) * 1000;
  activity_threshold = static_cast<int64_t>(activity) * 1000;
  idle_threshold = static_cast<int64_t>(idle) * 1000;

  this->sensitivity = sensitivity;

  // The easy way out.
  state = ACTIVITY_MONITOR_IDLE;
}

//! Sets the operation parameters.
void
LocalActivityMonitor::get_parameters(int &noise, int &activity, int &idle, int &sensitivity) const
{
  noise = static_cast<int>(noise_threshold / 1000);
  activity = static_cast<int>(activity_threshold / 1000);
  idle = static_cast<int>(idle_threshold / 1000);
  sensitivity = this->sensitivity;
}

//! Sets the callback listener.
void
LocalActivityMonitor::set_listener(IActivityMonitorListener::Ptr l)
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

  switch (state)
    {
    case ACTIVITY_MONITOR_IDLE:
    case ACTIVITY_MONITOR_FORCED_IDLE:
      {
        first_action_time = now;
        last_action_time = now;

        if (activity_threshold == 0)
          {
            state = ACTIVITY_MONITOR_ACTIVE;
          }
        else
          {
            state = ACTIVITY_MONITOR_NOISE;
          }
      }
      break;

    case ACTIVITY_MONITOR_NOISE:
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
                state = ACTIVITY_MONITOR_ACTIVE;
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
  IActivityMonitorListener::Ptr l;

  lock.lock();
  l = listener;
  lock.unlock();

  if (l)
    {
      // Listener is set.
      if (!l->action_notify())
        {
          // Remove listener.
          lock.lock();
          listener.reset();
          lock.unlock();
        }
    }
}

//! Loads the configuration of the monitor.
void
LocalActivityMonitor::load_config()
{
  TRACE_ENTRY();
  int noise = CoreConfig::monitor_noise()();
  int activity = CoreConfig::monitor_activity()();
  int idle = CoreConfig::monitor_idle()();
  int sensitivity = CoreConfig::monitor_sensitivity()();

  // Pre 1.0 compatibility...
  if (noise < 50)
    {
      noise *= 1000;
      CoreConfig::monitor_noise().set(noise);
    }

  if (activity < 50)
    {
      activity *= 1000;
      CoreConfig::monitor_activity().set(noise);
    }

  if (idle < 50)
    {
      idle *= 1000;
      CoreConfig::monitor_idle().set(noise);
    }

  TRACE_MSG("Monitor config = {} {} {} {}", noise, activity, idle, sensitivity);

  set_parameters(noise, activity, idle, sensitivity);
}

// TODO: implement somewhere else:

// //! Computes the current state.
// void
// LocalActivityMonitor::heartbeat()
// {
//   // Default
//   ActivityState state = local_monitor->get_state();
//   int64_t current_time = TimeSource::get_monotonic_time_sec();

//   map<std::string, int64_t>::iterator i = external_activity.begin();
//   while (i != external_activity.end())
//     {
//       map<std::string, int64_t>::iterator next = i;
//       next++;

//       if (i->second >= current_time)
//         {
//           state = ACTIVITY_ACTIVE;
//         }
//       else
//         {
//           external_activity.erase(i);
//         }

//       i = next;
//     }

//   if (local_state != state)
//     {
//       local_state = state;
//     }

//   monitor_state = state;
// }

// void
// LocalActivityMonitor::report_external_activity(std::string who, bool act)
// {
//   TRACE_ENTRY_PAR(who, act);
//   if (act)
//     {
//       external_activity[who] = TimeSource::get_monotonic_time_sec() + 10;
//     }
//   else
//     {
//       external_activity.erase(who);
//     }
//   // }
