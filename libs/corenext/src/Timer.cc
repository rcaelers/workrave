// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include "Timer.hh"

#include <memory>

#include "debug.hh"
#include "utils/TimeSource.hh"

#include <sstream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <utility>

#include "TimePred.hh"

using namespace std;
using namespace workrave::utils;

Timer::Timer(std::string id)
  : timer_enabled(false)
  , timer_frozen(false)
  , timer_state(STATE_INVALID)
  , snooze_interval(60)
  , snooze_inhibited(false)
  , limit_enabled(true)
  , limit_interval(600)
  , auto_reset_enabled(true)
  , auto_reset_interval(120)
  , daily_auto_reset(nullptr)
  , elapsed_timespan(0)
  , elapsed_timespan_at_last_limit(0)
  , elapsed_idle_timespan(0)
  , total_overdue_timespan(0)
  , last_start_time(0)
  , last_stop_time(0)
  , last_reset_time(0)
  , last_daily_reset_time(0)
  , next_reset_time(0)
  , next_daily_reset_time(0)
  , next_limit_time(0)
  , timer_id(std::move(id))
{
}

Timer::~Timer()
{
  delete daily_auto_reset;
}

void
Timer::enable()
{
  TRACE_ENTRY_PAR(timer_id, timer_enabled);
  if (!timer_enabled)
    {
      timer_enabled = true;
      snooze_inhibited = false;
      stop_timer();

      if (is_auto_reset_enabled() && get_elapsed_time() == 0)
        {
          // Start with idle time at maximum.
          elapsed_idle_timespan = auto_reset_interval;
        }

      if (is_limit_enabled() && get_elapsed_time() >= limit_interval)
        {
          // Break is overdue, force a snooze.
          elapsed_timespan_at_last_limit = 0;
          compute_next_limit_time();
        }

      compute_next_reset_time();
      compute_next_daily_reset_time();
    }
}

void
Timer::disable()
{
  TRACE_ENTRY_PAR(timer_id, timer_enabled);

  if (timer_enabled)
    {
      timer_enabled = false;
      stop_timer();

      last_start_time = 0;
      last_stop_time = 0;
      last_reset_time = 0;
      next_limit_time = 0;
      next_reset_time = 0;

      timer_state = STATE_INVALID;
    }
}

void
Timer::snooze_timer()
{
  TRACE_ENTRY_PAR(timer_id);
  if (timer_enabled)
    {
      next_limit_time = 0;
      elapsed_timespan_at_last_limit = get_elapsed_time();
      compute_next_limit_time();
    }
}

//! Prevents 'limit reached' snoozing until timer reset.
void
Timer::inhibit_snooze()
{
  TRACE_ENTRY_PAR(timer_id);
  snooze_inhibited = true;
  compute_next_limit_time();
}

void
Timer::start_timer()
{
  TRACE_ENTRY_PAR(timer_id, timer_state);
  if (timer_state != STATE_RUNNING)
    {
      // Set last start and stop times.
      if (!timer_frozen)
        {
          // Timer is not frozen, so let's start.
          last_start_time = TimeSource::get_monotonic_time_sec_sync();
          elapsed_idle_timespan = 0;
        }
      else
        {
          TRACE_MSG("Timer is frozen");
          // The timer is frozen, so we don't start counting 'active' time.
          // Instead, update the elapsed idle time.
          if (last_stop_time != 0)
            {
              elapsed_idle_timespan += (TimeSource::get_monotonic_time_sec_sync() - last_stop_time);
            }
          last_start_time = 0;
        }

      // Reset values that are only used when the timer is not running.
      last_stop_time = 0;
      next_reset_time = 0;

      // update state.
      timer_state = STATE_RUNNING;

      // When to generate a limit-reached-event.
      compute_next_limit_time();
    }
}

void
Timer::stop_timer()
{
  TRACE_ENTRY_PAR(timer_id, timer_state, timer_state);
  if (timer_state != STATE_STOPPED)
    {
      // Update last stop time.
      last_stop_time = TimeSource::get_monotonic_time_sec_sync();

      // Update elapsed time.
      if (last_start_time != 0)
        {
          // But only if we are running...
          elapsed_timespan += (last_stop_time - last_start_time);
        }

      // Reset last start time.
      last_start_time = 0;
      next_limit_time = 0;

      // Update state.
      timer_state = STATE_STOPPED;

      // When to reset the timer.
      compute_next_reset_time();
    }
}

void
Timer::reset_timer()
{
  TRACE_ENTRY_PAR(timer_id, timer_state);

  // Update total overdue.
  int64_t elapsed = get_elapsed_time();
  if (is_limit_enabled() && elapsed > limit_interval)
    {
      total_overdue_timespan += (elapsed - limit_interval);
    }

  // Full reset.
  elapsed_timespan = 0;
  elapsed_timespan_at_last_limit = 0;
  last_reset_time = TimeSource::get_monotonic_time_sec_sync();
  snooze_inhibited = false;

  if (timer_state == STATE_RUNNING)
    {
      // The timer is reset while running, Pretend the timer just started.
      last_start_time = TimeSource::get_monotonic_time_sec_sync();
      last_stop_time = 0;
      next_reset_time = 0;

      compute_next_limit_time();
      elapsed_idle_timespan = 0;
    }
  else
    {
      // The timer is reset while it is not running.
      last_start_time = 0;
      next_reset_time = 0;
      next_limit_time = 0;

      if (is_auto_reset_enabled())
        {
          elapsed_idle_timespan = auto_reset_interval;
          last_stop_time = TimeSource::get_monotonic_time_sec_sync();
        }
    }

  next_daily_reset_time = 0;
  compute_next_daily_reset_time();
}

void
Timer::freeze_timer(bool freeze)
{
  TRACE_ENTRY_PAR(timer_id, freeze, timer_enabled);

  if (timer_enabled)
    {
      if (freeze && !timer_frozen)
        {
          TRACE_MSG("freezing");
          // freeze timer.
          if (last_start_time != 0 && timer_state == STATE_RUNNING)
            {
              elapsed_timespan += (TimeSource::get_monotonic_time_sec_sync() - last_start_time);
              last_start_time = 0;
            }
        }
      else if (!freeze && timer_frozen)
        {
          TRACE_MSG("unfreezing");
          // defrost timer.
          if (timer_state == STATE_RUNNING)
            {
              last_start_time = TimeSource::get_monotonic_time_sec_sync();
              elapsed_idle_timespan = 0;

              compute_next_limit_time();
            }
        }
    }

  timer_frozen = freeze;
}

TimerEvent
Timer::process(bool user_is_active)
{
  TRACE_ENTRY_PAR(user_is_active);

  int64_t current_time = TimeSource::get_monotonic_time_sec_sync();
  TimerEvent event = TIMER_EVENT_NONE;

  if (timer_enabled)
    {
      if (user_is_active && timer_state != STATE_RUNNING)
        {
          start_timer();
        }
      else if (!user_is_active && timer_state == STATE_RUNNING)
        {
          stop_timer();
        }
    }

  if ((daily_auto_reset != nullptr) && next_daily_reset_time != 0
      && TimeSource::get_real_time_sec_sync() >= next_daily_reset_time)
    {
      TRACE_MSG("daily reset");
      // A next reset time was set and the current time >= reset time.
      // So reset the timer and send a reset event.
      reset_timer();

      last_daily_reset_time = TimeSource::get_real_time_sec_sync();
      next_daily_reset_time = 0;

      compute_next_daily_reset_time();
      event = TIMER_EVENT_RESET;
    }
  else if (next_limit_time != 0 && current_time >= next_limit_time)
    {
      TRACE_MSG("limit");
      // A next limit time was set and the current time >= limit time.
      next_limit_time = 0;
      elapsed_timespan_at_last_limit = get_elapsed_time();

      compute_next_limit_time();
      event = TIMER_EVENT_LIMIT_REACHED;
    }
  else if (next_reset_time != 0 && current_time >= next_reset_time)
    {
      TRACE_MSG("reset");
      bool natural = is_limit_enabled() && limit_interval >= get_elapsed_time();

      // A next reset time was set and the current time >= reset time.
      next_reset_time = 0;
      reset_timer();

      event = natural ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_RESET;
      if (natural)
        {
          TRACE_MSG("natural");
        }
    }

  TRACE_VAR(event);
  return event;
}

int64_t
Timer::get_elapsed_time() const
{
  int64_t ret = elapsed_timespan;

  if (timer_enabled && last_start_time != 0)
    {
      ret += (TimeSource::get_monotonic_time_sec_sync() - last_start_time);
    }

  return ret;
}

int64_t
Timer::get_elapsed_idle_time() const
{
  int64_t ret = elapsed_idle_timespan;

  if (timer_enabled && last_stop_time != 0)
    {
      ret += (TimeSource::get_monotonic_time_sec_sync() - last_stop_time);
    }

  return ret;
}

bool
Timer::is_running() const
{
  return timer_state == STATE_RUNNING;
}

bool
Timer::is_enabled() const
{
  return timer_enabled;
}

void
Timer::set_auto_reset(int reset_time)
{
  if (reset_time > auto_reset_interval)
    {
      // increasing reset_time, re-enable limit-reached snoozing.
      snooze_inhibited = false;
    }

  auto_reset_interval = reset_time;
  compute_next_reset_time();
}

void
Timer::set_daily_reset(TimePred *predicate)
{
  delete daily_auto_reset;
  daily_auto_reset = predicate;
  compute_next_daily_reset_time();
}

void
Timer::set_auto_reset_enabled(bool b)
{
  auto_reset_enabled = b;
  compute_next_reset_time();
}

bool
Timer::is_auto_reset_enabled() const
{
  return auto_reset_enabled && auto_reset_interval > 0;
}

int64_t
Timer::get_auto_reset() const
{
  return auto_reset_interval;
}

int64_t
Timer::get_next_reset_time() const
{
  return next_reset_time;
}

void
Timer::set_limit(int limit_time)
{
  limit_interval = limit_time;

  if (get_elapsed_time() < limit_time)
    {
      // limit increased, pretend there was no limit-reached yet.
      elapsed_timespan_at_last_limit = 0;
    }

  compute_next_limit_time();
}

void
Timer::set_limit_enabled(bool b)
{
  if (limit_enabled != b)
    {
      limit_enabled = b;
      compute_next_limit_time();
    }
}

bool
Timer::is_limit_enabled() const
{
  return limit_enabled && limit_interval > 0;
}

int64_t
Timer::get_limit() const
{
  return limit_interval;
}

int64_t
Timer::get_next_limit_time() const
{
  return next_limit_time;
}

void
Timer::set_snooze(int64_t t)
{
  snooze_interval = t;
}

int64_t
Timer::get_snooze() const
{
  return snooze_interval;
}

std::string
Timer::get_id() const
{
  return timer_id;
}

std::string
Timer::serialize_state() const
{
  stringstream ss;

  ss << timer_id << " " << TimeSource::get_real_time_sec_sync() << " " << get_elapsed_time() << " " << last_daily_reset_time
     << " " << total_overdue_timespan << " " << snooze_inhibited << " " << 0 << " " << elapsed_timespan_at_last_limit << " "
     << 0 /* timezone */;

  return ss.str();
}

bool
Timer::deserialize_state(const std::string &state, int version)
{
  TRACE_ENTRY();
  istringstream ss(state);

  int64_t save_time = 0;
  int64_t elapsed = 0;
  int64_t last_reset = 0;
  int64_t overdue = 0;
  int64_t llt = 0;
  int64_t lle = 0;
  bool si = false;

  ss >> save_time >> elapsed >> last_reset >> overdue >> si >> llt >> lle;

  if (version == 3)
    {
      // Ignored.
      int64_t tz = 0;
      ss >> tz;
    }

  // Sanity check...
  if (last_reset > save_time)
    {
      last_reset = save_time;
    }

  TRACE_VAR(si, llt, lle);
  TRACE_VAR(snooze_inhibited);

  last_daily_reset_time = last_reset;
  total_overdue_timespan = overdue;
  elapsed_timespan = 0;
  last_start_time = 0;
  last_stop_time = 0;

  bool tooOld = (is_auto_reset_enabled() && (TimeSource::get_real_time_sec_sync() - save_time > auto_reset_interval));

  if (!tooOld)
    {
      if (is_auto_reset_enabled())
        {
          next_reset_time = TimeSource::get_monotonic_time_sec_sync() + auto_reset_interval;
        }
      elapsed_timespan = elapsed;
      snooze_inhibited = si;
    }

  // overdue, so snooze
  if (is_limit_enabled() && get_elapsed_time() >= limit_interval)
    {
      elapsed_timespan_at_last_limit = lle;
      compute_next_limit_time();
    }

  compute_next_daily_reset_time();

  TRACE_MSG("elapsed = {}", elapsed_timespan);
  return true;
}

// void
// Timer::set_state(int elapsed, int idle, int overdue)
// {
//   TRACE_ENTRY_PAR(elapsed, idle, overdue);

//   elapsed_timespan = elapsed;
//   elapsed_idle_timespan = idle;

//   if (last_start_time != 0)
//     {
//       last_start_time = TimeSource::get_monotonic_time_sec_sync();
//     }

//   if (last_stop_time != 0)
//     {
//       last_stop_time = TimeSource::get_monotonic_time_sec_sync();
//     }

//   if (elapsed_idle_timespan > auto_reset_interval && is_auto_reset_enabled())
//     {
//       elapsed_idle_timespan = auto_reset_interval;
//     }

//   if (overdue != -1)
//     {
//       total_overdue_timespan = overdue;
//       if (is_limit_enabled() && get_elapsed_time() > limit_interval)
//         {
//           total_overdue_timespan -= (get_elapsed_time() - limit_interval);
//         }
//     }

//   compute_next_reset_time();
//   compute_next_limit_time();
//   compute_next_daily_reset_time();

//   // }

int64_t
Timer::get_total_overdue_time() const
{
  int64_t ret = total_overdue_timespan;
  int64_t elapsed = get_elapsed_time();

  if (is_limit_enabled() && elapsed > limit_interval)
    {
      ret += (elapsed - limit_interval);
    }

  return ret;
}

void
Timer::daily_reset()
{
  total_overdue_timespan = 0;
}

void
Timer::compute_next_limit_time()
{
  TRACE_ENTRY_PAR(timer_id);

  // default action. No next limit.
  next_limit_time = 0;

  if (timer_enabled)
    {
      if (timer_state == STATE_RUNNING && last_start_time != 0 && is_limit_enabled())
        {
          // The timer is running and a limit != 0 is set.

          if (get_elapsed_time() >= limit_interval)
            {
              // The timer already reached its limit. We need to re-send the
              // limit-reached event after 'snooze_interval' seconds of
              // activity after the previous event. Unless snoozing is
              // inhibted. This is dependent of user activity.
              if (!snooze_inhibited)
                {
                  next_limit_time = (last_start_time - elapsed_timespan + elapsed_timespan_at_last_limit + snooze_interval);
                }
              TRACE_MSG("Next limit time (1) = {} {}", next_limit_time, (next_limit_time - TimeSource::get_real_time_sec_sync()));
            }
          else
            {
              // The timer did not yet reaches its limit.
              // new limit = last start time + limit - elapsed.
              next_limit_time = last_start_time + limit_interval - elapsed_timespan;
              TRACE_MSG("Next limit time (2) = {} {}", next_limit_time, (next_limit_time - TimeSource::get_real_time_sec_sync()));
            }
        }
    }
}

void
Timer::compute_next_reset_time()
{
  TRACE_ENTRY_PAR(timer_id);

  // default action. No next reset.
  next_reset_time = 0;

  if (timer_enabled && timer_state == STATE_STOPPED && last_stop_time != 0 && is_auto_reset_enabled())
    {
      // We are enabled, not running and a reset time != 0 was set.

      // next reset time = last stop time + auto reset
      next_reset_time = last_stop_time + auto_reset_interval - elapsed_idle_timespan;
      TRACE_MSG("Next reset time = {} {}", next_reset_time, (next_reset_time - TimeSource::get_real_time_sec_sync()));
      if (next_reset_time <= last_reset_time || next_reset_time <= last_stop_time)
        {
          // Just is sanity check, can't reset before the previous one..
          next_reset_time = 0;
          TRACE_MSG("Next reset time in past, setting to 0 ");
        }
    }
}

void
Timer::compute_next_daily_reset_time()
{
  // This one ALWAYS sends a reset, also when the timer is disabled.

  if (daily_auto_reset != nullptr)
    {
      if (last_daily_reset_time == 0)
        {
          // The timer did not reach a predicate reset before. Just take
          // the current time as the last reset time...
          last_daily_reset_time = TimeSource::get_real_time_sec_sync();
        }

      next_daily_reset_time = daily_auto_reset->get_next(last_daily_reset_time);
    }
}
