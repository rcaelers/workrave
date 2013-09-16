// Timer.cc --- break timer
//
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
#include "config.h"
#endif

#include "Timer.hh"

#include <boost/shared_ptr.hpp>

#include "debug.hh"
#include "utils/TimeSource.hh"

#include <sstream>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "DayTimePred.hh"
#include "timeutil.h"

using namespace std;
using namespace workrave::utils;

Timer::Ptr
Timer::create(const std::string &id)
{
  return Ptr(new Timer(id));
}


//! Constructs a new break timer.
Timer::Timer(const std::string &id) :
  timer_enabled(false),
  timer_frozen(false),
  timer_state(STATE_INVALID),
  snooze_interval(60),
  snooze_inhibited(false),
  limit_enabled(true),
  limit_interval(600),
  autoreset_enabled(true),
  autoreset_interval(120),
  daily_autoreset(NULL),
  elapsed_timespan(0),
  elapsed_timespan_at_last_limit(0),
  elapsed_idle_timespan(0),
  total_overdue_timespan(0),
  last_start_time(0),
  last_stop_time(0),
  last_reset_time(0),
  last_daily_reset_time(0),
  next_reset_time(0),
  next_daily_reset_time(0),
  next_limit_time(0),
  timer_id(id)
{
}


//! Destructor
Timer::~Timer()
{
  delete daily_autoreset;
}


//! Enables the timer.
void
Timer::enable()
{
  TRACE_ENTER_MSG("Timer::enable", timer_id << " " << timer_enabled);

  if (!timer_enabled)
    {
      timer_enabled = true;
      snooze_inhibited = false;
      stop_timer();

      if (autoreset_enabled && autoreset_interval != 0 && get_elapsed_time() == 0)
        {
          // Start with idle time at maximum.
          elapsed_idle_timespan = autoreset_interval;
        }

      if (limit_enabled && get_elapsed_time() >= limit_interval)
        {
          // Break is overdue, force a snooze.
          elapsed_timespan_at_last_limit = 0;
          compute_next_limit_time();
        }

      compute_next_reset_time();
      compute_next_daily_reset_time();
    }

  TRACE_EXIT();
}


//! Disables the timer.
void
Timer::disable()
{
  TRACE_ENTER_MSG("Timer::disable", timer_id << " " << timer_enabled);

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
  
  TRACE_EXIT();
}


//! Snoozes the timer.
void
Timer::snooze_timer()
{
  TRACE_ENTER_MSG("Timer::snooze_timer", timer_id);

  if (timer_enabled)
    {
      next_limit_time = 0;
      elapsed_timespan_at_last_limit = get_elapsed_time();
      compute_next_limit_time();
    }
  
  TRACE_EXIT();
}


//! Prevents 'limit reached' snoozing until timer reset.
void
Timer::inhibit_snooze()
{
  TRACE_ENTER_MSG("Timer::inhibit_snooze", timer_id);
  snooze_inhibited = true;
  compute_next_limit_time();
  TRACE_EXIT();
}


//! Starts the timer.
void
Timer::start_timer()
{
  TRACE_ENTER_MSG("Timer::start_timer", timer_id << " " << timer_state);

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
  TRACE_EXIT();
}


//! Stops the timer.
void
Timer::stop_timer()
{
  TRACE_ENTER_MSG("Timer::stop_timer", timer_id << " " << timer_state << " " << timer_state);

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

      // Update state.
      timer_state = STATE_STOPPED;

      // When to reset the timer.
      compute_next_reset_time();
    }
  TRACE_EXIT();
}


//! Resets and stops the timer.
void
Timer::reset_timer()
{
  TRACE_ENTER_MSG("Timer::reset", timer_id << " " << timer_state);

  // Update total overdue.
  int64_t elapsed = get_elapsed_time();
  if (limit_enabled && elapsed > limit_interval)
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

      compute_next_limit_time();
      next_reset_time = 0;
      elapsed_idle_timespan = 0;
    }
  else
    {
      // The timer is reset while it is not running.
      last_start_time = 0;
      next_reset_time = 0;
      next_limit_time = 0;

      if (autoreset_enabled && autoreset_interval != 0)
        {
          elapsed_idle_timespan = autoreset_interval;
          last_stop_time = TimeSource::get_monotonic_time_sec_sync();
        }
    }

  next_daily_reset_time = 0;
  compute_next_daily_reset_time();
  TRACE_EXIT();
}


//! Freezes the elapsed time of the timer.
void
Timer::freeze_timer(bool freeze)
{
  TRACE_ENTER_MSG("Timer::freeze_timer",
                  timer_id << freeze << " " <<
                  timer_enabled << " ");

  if (timer_enabled)
    {
      if (freeze && !timer_frozen)
        {
          // freeze timer.
          if (last_start_time != 0 && timer_state == STATE_RUNNING)
            {
              elapsed_timespan += (TimeSource::get_monotonic_time_sec_sync() - last_start_time);
              last_start_time = 0;
            }
        }
      else if (!freeze && timer_frozen)
        {
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
  TRACE_EXIT();
}


//! Perform timer processing.
TimerEvent
Timer::process(bool user_is_active)
{
  TRACE_ENTER_MSG("Timer::process", user_is_active);
  
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

  if (daily_autoreset &&
      next_daily_reset_time != 0 &&
      TimeSource::get_real_time_sec_sync() >= next_daily_reset_time)
    {
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
      // A next limit time was set and the current time >= limit time.
      next_limit_time = 0;
      elapsed_timespan_at_last_limit = get_elapsed_time();

      compute_next_limit_time();
      event = TIMER_EVENT_LIMIT_REACHED;
    }
  else if (next_reset_time != 0 && current_time >=  next_reset_time)
    {
      // A next reset time was set and the current time >= reset time.
      next_reset_time = 0;
      reset_timer();

      bool natural = limit_enabled && limit_interval >= get_elapsed_time();
      event = natural ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_RESET;
    }

  TRACE_RETURN(event);
  return event;
}


//! Returns the elapsed time.
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


//! Returns the elapsed idle time.
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


//! Returns the timer state.
TimerState
Timer::get_state() const
{
  return timer_state;
}


//! Returns the enabled state.
bool
Timer::is_enabled() const
{
  return timer_enabled;
}


//! Sets auto-reset time period.
void
Timer::set_auto_reset(int reset_time)
{
  if (reset_time > autoreset_interval)
    {
      // increasing reset_time, re-enable limit-reached snoozing.
      snooze_inhibited = false;
    }

  autoreset_interval = reset_time;
  compute_next_reset_time();
}


//! Sets the auto-reset predicate.
void
Timer::set_daily_reset(DayTimePred *predicate)
{
  delete daily_autoreset;
  daily_autoreset = predicate;
  compute_next_daily_reset_time();
}



//! Enables or disables auto-reset.
void
Timer::set_auto_reset_enabled(bool b)
{
  autoreset_enabled = b;
  compute_next_reset_time();
}



//! Does this timer have an auto reset?
bool
Timer::is_auto_reset_enabled() const
{
  return autoreset_enabled;
}


//! Returns the auto reset interval.
int64_t
Timer::get_auto_reset() const
{
  return autoreset_interval;
}


//! Returns the time the timer will reset.
int64_t
Timer::get_next_reset_time() const
{
  return next_reset_time;
}


//! Sets the limit time in seconds.
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

//! Enables or disables limiting
void
Timer::set_limit_enabled(bool b)
{
  if (limit_enabled != b)
    {
      limit_enabled = b;
      compute_next_limit_time();
    }
}


//! Does this timer have a limit set?
bool
Timer::is_limit_enabled() const
{
  return limit_enabled;
}


//! Returns the limit.
int64_t
Timer::get_limit() const
{
  return limit_interval;
}


//! Returns the time the limit will be reached.
int64_t
Timer::get_next_limit_time() const
{
  return next_limit_time;
}


//! Sets the snooze interval of the timer.
void
Timer::set_snooze(int64_t t)
{
  snooze_interval = t;
}


//! Returns the snooze interval.
int64_t
Timer::get_snooze() const
{
  return snooze_interval;
}

//! Gets ID of this timer.
std::string
Timer::get_id() const
{
  return timer_id;
}

std::string
Timer::serialize_state() const
{
  stringstream ss;

  ss << timer_id << " "
     << TimeSource::get_real_time_sec_sync() << " "
     << get_elapsed_time() << " "
     << last_daily_reset_time << " "
     << total_overdue_timespan << " "
     << snooze_inhibited << " "
     << 0 << " "
     << elapsed_timespan_at_last_limit << " "
     << 0 /* timezone */;

  return ss.str();
}

bool
Timer::deserialize_state(const std::string &state, int version)
{
  TRACE_ENTER("Timer::deserialize_state");
  istringstream ss(state);

  int64_t save_time = 0;
  int64_t elapsed = 0;
  int64_t last_reset = 0;
  int64_t overdue = 0;
  int64_t llt = 0;
  int64_t lle = 0;
  bool si = false;

  ss >> save_time
     >> elapsed
     >> last_reset
     >> overdue
     >> si
     >> llt
     >> lle;

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

  TRACE_MSG(si << " " << llt << " " << lle);
  TRACE_MSG(snooze_inhibited);

  last_daily_reset_time = last_reset;
  total_overdue_timespan = overdue;
  elapsed_timespan = 0;
  last_start_time = 0;
  last_stop_time = 0;

  bool tooOld = ((autoreset_enabled && autoreset_interval != 0) && (TimeSource::get_real_time_sec_sync() - save_time >  autoreset_interval));

  if (! tooOld)
    {
      if (autoreset_enabled)
        {
          next_reset_time = TimeSource::get_monotonic_time_sec_sync() + autoreset_interval;
        }
      elapsed_timespan = elapsed;
      snooze_inhibited = si;
    }

  // overdue, so snooze
  if (limit_enabled && get_elapsed_time() >= limit_interval)
    {
      elapsed_timespan_at_last_limit = lle;

      compute_next_limit_time();
    }

  compute_next_daily_reset_time();

  TRACE_MSG("elapsed = " << elapsed_timespan);
  return true;
}

// void
// Timer::set_state(int elapsed, int idle, int overdue)
// {
//   TRACE_ENTER_MSG("Timer::set_state", elapsed << " " << idle << " " << overdue);

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

//   if (elapsed_idle_timespan > autoreset_interval && autoreset_enabled)
//     {
//       elapsed_idle_timespan = autoreset_interval;
//     }

//   if (overdue != -1)
//     {
//       total_overdue_timespan = overdue;
//       if (limit_enabled && get_elapsed_time() > limit_interval)
//         {
//           total_overdue_timespan -= (get_elapsed_time() - limit_interval);
//         }
//     }

//   compute_next_reset_time();
//   compute_next_limit_time();
//   compute_next_daily_reset_time();

//   TRACE_EXIT();
// }


//! Returns the total overdue time of the timer.
int64_t
Timer::get_total_overdue_time() const
{
  int64_t ret = total_overdue_timespan;
  int64_t elapsed = get_elapsed_time();

  if (limit_enabled && elapsed > limit_interval)
    {
      ret += (elapsed - limit_interval);
    }

  return ret;
}


//! Daily Reset.
void
Timer::daily_reset_timer()
{
  total_overdue_timespan = 0;
}


//! Computes the time the limit will be reached.
void
Timer::compute_next_limit_time()
{
  TRACE_ENTER_MSG("Timer::compute_next_limit_time", timer_id);
  // default action. No next limit.
  next_limit_time = 0;

  if (timer_enabled)
    {
      if (timer_state == STATE_RUNNING && last_start_time > 0 &&
          limit_enabled && limit_interval != 0)
        {
          // The timer is running and a limit != 0 is set.

          if (get_elapsed_time() > limit_interval)
            {
              // The timer already reached its limit. We need to re-send the
              // limit-reached event after 'snooze_interval' seconds of
              // activity after the previous event. Unless snoozing is
              // inhibted. This is dependent of user activity.
              if (!snooze_inhibited)
                {
                  next_limit_time = (last_start_time - elapsed_timespan +
                                     elapsed_timespan_at_last_limit + snooze_interval);
                }
            }
          else
            {
              // The timer did not yet reaches its limit.
              // new limit = last start time + limit - elapsed.
              next_limit_time = last_start_time + limit_interval - elapsed_timespan;
            }
        }
    }
  TRACE_EXIT();
}


//! Computes the time the auto-reset must take place.
void
Timer::compute_next_reset_time()
{
  // default action. No next reset.
  next_reset_time = 0;

  if (timer_enabled && timer_state == STATE_STOPPED &&
      last_stop_time != 0 &&
      autoreset_enabled && autoreset_interval != 0)
    {
      // We are enabled, not running and a reset time != 0 was set.

      // next reset time = last stop time + auto reset
      next_reset_time = last_stop_time + autoreset_interval - elapsed_idle_timespan;

      if (next_reset_time <= last_reset_time ||
          next_reset_time <= last_stop_time)
        {
          // Just is sanity check, can't reset before the previous one..
          next_reset_time = 0;
        }
    }
}


void
Timer::compute_next_daily_reset_time()
{
  // This one ALWAYS sends a reset, also when the timer is disabled.

  if (daily_autoreset)
    {
      if (last_daily_reset_time == 0)
        {
          // The timer did not reach a predicate reset before. Just take
          // the current time as the last reset time...
          last_daily_reset_time = TimeSource::get_real_time_sec_sync();
        }

      next_daily_reset_time = daily_autoreset->get_next(last_daily_reset_time);
    }
}
