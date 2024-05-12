// Copyright (C) 2001 - 2012 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <sstream>

#include "Timer.hh"
#include "TimePredFactory.hh"
#include "TimePred.hh"
#include "utils/TimeSource.hh"

using namespace std;
using namespace workrave;
using namespace workrave::utils;

//! Constructs a new break timer.
/*!
 *  \param time_source source of the current time that will be used by the timer.
 */
Timer::Timer(const std::string &timer_id)
  : timer_id(timer_id)
  , timer_enabled{timer_id + ".timer.enabled", false}
  , timer_frozen{timer_id + ".timer.frozen", false}
  , activity_state{timer_id + ".timer.activity_state", ACTIVITY_UNKNOWN}
  , timer_state{timer_id + ".timer.state", STATE_INVALID}
  , activity_sensitive{timer_id + ".timer.activity_sensitive", true}
  , insensitive_mode{timer_id + ".timer.insensitive_mode", INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED}
{
}

//! Destructor
Timer::~Timer()
{
  delete autoreset_interval_predicate;
  delete activity_monitor;
}

//! Enables the timer.
void
Timer::enable()
{
  TRACE_ENTRY_PAR(timer_id, timer_enabled);

  if (!timer_enabled)
    {
      timer_enabled = true;
      snooze_inhibited = false;
      snooze_on_active = true;
      stop_timer();

      if (autoreset_enabled && autoreset_interval != 0 && get_elapsed_time() == 0)
        {
          // Start with idle time at maximum.
          elapsed_idle_time = autoreset_interval;
        }

      if (limit_enabled && get_elapsed_time() >= limit_interval)
        {
          // Break is overdue, force a snooze.
          last_limit_time = TimeSource::get_real_time_sec_sync();
          last_limit_elapsed = 0;
          compute_next_limit_time();
        }

      compute_next_predicate_reset_time();
    }
}

//! Disables the timer.
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

//! Enables or disables limiting
/*!
 * \param b indicates whether the timer limit must be enabled (\c true) or
 *          disabled (\c false).
 */
void
Timer::set_limit_enabled(bool b)
{
  if (limit_enabled != b)
    {
      limit_enabled = b;
      compute_next_limit_time();
    }
}

//! Sets the limit time in seconds.
/*!
 * \param limit_time time at which this timer reaches its limit.
 *
 */
void
Timer::set_limit(int limit_time)
{
  limit_interval = limit_time;

  if (get_elapsed_time() < limit_time)
    {
      // limit increased, pretend there was no limit-reached yet.
      last_limit_time = 0;
      last_limit_elapsed = 0;
    }

  compute_next_limit_time();
}

//! Enables or disables auto-reset.
/*!
 * \param b indicates whether the timer auto-reset must be enabled (\c true)
 *          or disabled (\c false).
 */
void
Timer::set_auto_reset_enabled(bool b)
{
  autoreset_enabled = b;
  compute_next_reset_time();
}

//! Sets auto-reset time period.
/*!
 * \param resetTime after this amount of idle time the timer will reset itself.
 */
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
/*!
 * \param predicate auto-reset predicate.
 */
void
Timer::set_auto_reset(string predicate)
{
  delete autoreset_interval_predicate;
  autoreset_interval_predicate = TimePredFactory::create_time_pred(predicate);
  compute_next_predicate_reset_time();
}

void
Timer::set_auto_reset(TimePred *pred)
{
  delete autoreset_interval_predicate;
  autoreset_interval_predicate = pred;
  compute_next_predicate_reset_time();
}

//! Sets the snooze interval of the timer.
void
Timer::set_snooze_interval(int64_t t)
{
  snooze_interval = t;
}

//! Prevents 'limit reached' snoozing until timer reset.
void
Timer::inhibit_snooze()
{
  snooze_inhibited = true;
}

//! Marks timer sensitive or insensitive for activity
void
Timer::set_activity_sensitive(bool a)
{
  TRACE_ENTRY_PAR(a);

  activity_sensitive = a;
  activity_state = ACTIVITY_UNKNOWN;

  if (!activity_sensitive)
    {
      // If timer is made insensitive, start it if
      // it has some elasped time. Otherwise a daily limit
      // will never start (well, not until it resets...)
      int64_t elasped = get_elapsed_time();
      if (elasped > 0 && (elasped < limit_interval || !limit_enabled))
        {
          activity_state = ACTIVITY_ACTIVE;
        }
    }
}

//! Sets the activity insensitive mode
void
Timer::set_insensitive_mode(InsensitiveMode mode)
{
  TRACE_ENTRY_PAR(mode);
  insensitive_mode = mode;
}

//! Forces a activity insensitive timer to become idle
void
Timer::force_idle()
{
  TRACE_ENTRY();
  if (!activity_sensitive)
    {
      TRACE_MSG("Forcing idle");
      activity_state = ACTIVITY_IDLE;
    }
}

//! Forces a activity insensitive timer to become active
void
Timer::force_active()
{
  TRACE_ENTRY();
  if (!activity_sensitive)
    {
      TRACE_MSG("Forcing active");
      activity_state = ACTIVITY_ACTIVE;
    }
}

//! Computes the time the limit will be reached.
void
Timer::compute_next_limit_time()
{
  // default action. No next limit.
  next_limit_time = 0;

  if (timer_enabled)
    {
      if (last_limit_time > 0 && !snooze_on_active)
        {
          // The timer already reached its limit. We need to re-send the
          // limit-reached event 'snooze_interval' seconds after the previous
          // event. Unless snoozing is inhibted. This is independent of
          // user activity.

          if (!snooze_inhibited)
            {
              next_limit_time = last_limit_time + snooze_interval;
            }
        }
      else if (timer_state == STATE_RUNNING && last_start_time != 0 && limit_enabled && limit_interval != 0)
        {
          // The timer is running and a limit != 0 is set.

          if (last_limit_time > 0)
            {
              // The timer already reached its limit. We need to re-send the
              // limit-reached event after 'snooze_interval' seconds of
              // activity after the previous event. Unless snoozing is
              // inhibted. This is dependent of user activity.
              if (snooze_on_active && !snooze_inhibited)
                {
                  next_limit_time = (last_start_time - elapsed_time + last_limit_elapsed + snooze_interval);
                }
            }
          else
            {
              // The timer did not yet reaches its limit.
              // new limit = last start time + limit - elapsed.
              next_limit_time = last_start_time + limit_interval - elapsed_time;
            }
        }
    }
}

//! Computes the time the auto-reset must take place.
void
Timer::compute_next_reset_time()
{
  // default action. No next reset.
  next_reset_time = 0;

  if (timer_enabled && timer_state == STATE_STOPPED && last_stop_time != 0 && autoreset_enabled && autoreset_interval != 0)
    {
      // We are enabled, not running and a reset time != 0 was set.

      // next reset time = last stop time + auto reset
      next_reset_time = last_stop_time + autoreset_interval - elapsed_idle_time;

      if (next_reset_time <= last_reset_time)
        {
          // Just is sanity check, can't reset before the previous one..
          next_reset_time = 0;
        }
    }
}

void
Timer::compute_next_predicate_reset_time()
{
  // This one ALWAYS sends a reset, also when the timer is disabled.
  if (autoreset_interval_predicate)
    {
      if (last_pred_reset_time == 0)
        {
          // The timer did not reach a predicate reset before. Just take
          // the current time as the last reset time...
          last_pred_reset_time = TimeSource::get_real_time_sec_sync();
        }

      next_pred_reset_time = autoreset_interval_predicate->get_next(last_pred_reset_time);
    }
}

//! Daily Reset.
void
Timer::daily_reset_timer()
{
  total_overdue_time = 0;
}

//! Resets and stops the timer.
void
Timer::reset_timer()
{
  TRACE_ENTRY_PAR(timer_id, timer_state);

  // Update total overdue.
  int64_t elapsed = get_elapsed_time();
  if (limit_enabled && elapsed > limit_interval)
    {
      total_overdue_time += (elapsed - limit_interval);
    }

  // Full reset.
  elapsed_time = 0;
  last_limit_time = 0;
  last_limit_elapsed = 0;
  last_reset_time = TimeSource::get_real_time_sec_sync();
  snooze_inhibited = false;
  snooze_on_active = true;

  if (timer_state == STATE_RUNNING)
    {
      // The timer is reset while running, Pretend the timer just started.
      last_start_time = TimeSource::get_real_time_sec_sync();
      last_stop_time = 0;

      compute_next_limit_time();
      next_reset_time = 0;
      elapsed_idle_time = 0;
    }
  else
    {
      // The timer is reset while it is not running.
      last_start_time = 0;
      next_reset_time = 0;
      next_limit_time = 0;

      if (autoreset_enabled && autoreset_interval != 0)
        {
          elapsed_idle_time = autoreset_interval;
          last_stop_time = TimeSource::get_real_time_sec_sync();
        }
    }

  next_pred_reset_time = 0;
  compute_next_predicate_reset_time();
}

//! Starts the timer.
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
          last_start_time = TimeSource::get_real_time_sec_sync();
          elapsed_idle_time = 0;
        }
      else
        {
          TRACE_MSG("timer is frozen.");
          // The timer is frozen, so we don't start counting 'active' time.
          // Instead, update the elapsed idle time.
          if (last_stop_time != 0)
            {
              elapsed_idle_time += (TimeSource::get_real_time_sec_sync() - last_stop_time);
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

//! Stops the timer.
void
Timer::stop_timer()
{
  TRACE_ENTRY_PAR(timer_id, timer_state);

  if (timer_state != STATE_STOPPED)
    {
      TRACE_MSG("last_start_time = {}", last_start_time);

      // Update last stop time.
      last_stop_time = TimeSource::get_real_time_sec_sync();

      // Update elapsed time.
      if (last_start_time != 0)
        {
          // But only if we are running...
          elapsed_time += (last_stop_time - last_start_time);
        }

      TRACE_MSG("elapsed_idle_time = {}", elapsed_idle_time);
      TRACE_MSG("elapsed_time = {}", elapsed_time);

      // Reset last start time.
      last_start_time = 0;

      // Update state.
      timer_state = STATE_STOPPED;

      // When to reset the timer.
      compute_next_reset_time();

      // When to re-send a limit reached?
      // (in case of !snooze_on_active)
      compute_next_limit_time();
    }
}

//! Snoozes the timer.
/*!
 *  When the limit of this timer was reached, the limit reached event will be
 *  re-sent.
 */
void
Timer::snooze_timer()
{
  TRACE_ENTRY();
  if (timer_enabled)
    {
      // recompute.
      snooze_on_active = true;

      next_limit_time = 0;
      last_limit_time = TimeSource::get_real_time_sec_sync();
      last_limit_elapsed = get_elapsed_time();
      compute_next_limit_time();

      if (!activity_sensitive)
        {
          // Start the clock in case of insensitive timer.
          activity_state = ACTIVITY_ACTIVE;
        }
    }
}

//! Freezes the elapsed time of the timer.
void
Timer::freeze_timer(bool freeze)
{
  TRACE_ENTRY_PAR(timer_id, freeze, timer_frozen, timer_enabled, timer_state, activity_sensitive);

  // Only enabled activity sensitive timers care about freezing.
  if (timer_enabled && activity_sensitive)
    {
      if (freeze && !timer_frozen)
        {
          // freeze timer.
          if (last_start_time != 0 && timer_state == STATE_RUNNING)
            {
              TRACE_MSG("was started");
              elapsed_time += (TimeSource::get_real_time_sec_sync() - last_start_time);
              last_start_time = 0;
            }
        }
      else if (!freeze && timer_frozen)
        {
          // defrost timer.
          if (timer_state == STATE_RUNNING)
            {
              last_start_time = TimeSource::get_real_time_sec_sync();
              elapsed_idle_time = 0;

              compute_next_limit_time();
            }
        }
    }

  // test fix for Bug 746 -  Micro-break not counting down
  if (timer_enabled && !freeze && timer_frozen && timer_state == STATE_RUNNING && !last_start_time && !activity_sensitive)
    {
      TRACE_MSG("fix746");
      last_start_time = TimeSource::get_real_time_sec_sync();
      elapsed_idle_time = 0;
      compute_next_limit_time();
    }

  timer_frozen = freeze;
}

//! Returns the elapsed idle time.
int64_t
Timer::get_elapsed_idle_time() const
{
  TRACE_ENTRY();
  int64_t ret = elapsed_idle_time;

  if (timer_enabled && last_stop_time != 0)
    {
      ret += (TimeSource::get_real_time_sec_sync() - last_stop_time);
    }

  TRACE_VAR(ret);
  return ret;
}

//! Returns the elapsed time.
int64_t
Timer::get_elapsed_time() const
{
  TRACE_ENTRY_PAR(timer_id, timer_state);
  int64_t ret = elapsed_time;

  TRACE_VAR(ret, TimeSource::get_real_time_sec_sync(), last_start_time);

  if (timer_enabled && last_start_time != 0)
    {
      ret += (TimeSource::get_real_time_sec_sync() - last_start_time);
    }

  TRACE_VAR(ret);
  return ret;
}

//! Returns the total overdue time of the timer.
int64_t
Timer::get_total_overdue_time() const
{
  TRACE_ENTRY();
  int64_t ret = total_overdue_time;
  int64_t elapsed = get_elapsed_time();

  TRACE_VAR(ret, elapsed);

  if (limit_enabled && elapsed > limit_interval)
    {
      ret += (elapsed - limit_interval);
    }

  return ret;
}

//! Ajusts the timer when the system clock time changed.
void
Timer::shift_time(int delta)
{
  TRACE_ENTRY_PAR(delta);
  if (last_limit_time > 0)
    {
      last_limit_time += delta;
    }

  if (last_start_time > 0)
    {
      last_start_time += delta;
    }

  if (last_reset_time > 0)
    {
      last_reset_time += delta;
    }

  if (last_pred_reset_time > 0)
    {
      last_pred_reset_time += delta;
    }

  if (last_stop_time > 0)
    {
      last_stop_time += delta;
    }

  compute_next_limit_time();
  compute_next_reset_time();
  compute_next_predicate_reset_time();
}

//! Perform timer processing.
/*! \param new_activity_state the current activity state as reported by the
 *         (global) activity monitor.
 *  \param info returns the state of the timer.
 */
void
Timer::process(ActivityState new_activity_state, TimerInfo &info)
{
  TRACE_ENTRY_PAR(timer_id, timer_id, new_activity_state);

  int64_t current_time = TimeSource::get_real_time_sec_sync();

  // Default event to return.
  info.event = TIMER_EVENT_NONE;
  info.idle_time = get_elapsed_idle_time();
  info.elapsed_time = get_elapsed_time();

  TRACE_MSG("idle_time = {}", info.idle_time);
  TRACE_MSG("elapsed_tim = {}", info.elapsed_time);
  TRACE_MSG("enabled = {}", timer_enabled);
  TRACE_MSG("last_start_time {}", last_start_time);
  TRACE_MSG("next_pred_reset_time {}", next_pred_reset_time);
  TRACE_MSG("next_reset_time {}", next_reset_time);
  TRACE_MSG("time {}", current_time);
  TRACE_MSG("activity_sensitive {} {}", activity_sensitive, insensitive_mode);

  if (activity_monitor != nullptr)
    {
      // The timer uses its own activity monitor and ignores the 'global'
      // activity monitor state (ie. new_activity_state). So get the state
      // of the activity monitor used by this timer.
      new_activity_state = activity_monitor->get_current_state();
      TRACE_MSG("foreign activity state = {}", new_activity_state);
    }
  else if (activity_sensitive)
    {
      // This timer responds to the activity monitoring.
      TRACE_MSG("is activity sensitive");
    }
  else
    {
      // This timer is activity insensitive. It periodically switches between
      // idle and active.
      TRACE_MSG("is not activity sensitive");
      if (activity_state != ACTIVITY_UNKNOWN)
        {
          TRACE_MSG("activity_state = {} new_activity_state = {} elapsed_time = {}",
                    activity_state,
                    new_activity_state,
                    get_elapsed_time());

          if (insensitive_mode == INSENSITIVE_MODE_IDLE_ALWAYS)
            // Forces ACTIVITY_IDLE every time, regardless of sensitivity
            {
              TRACE_MSG("MODE_IDLE_ALWAYS: Forcing ACTIVITY_IDLE");
              new_activity_state = activity_state = ACTIVITY_IDLE;
            }

          if (activity_state == ACTIVITY_ACTIVE)
            {
              if (insensitive_mode == INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED)
                {
                  TRACE_MSG("MODE_IDLE_ON_LIMIT_REACHED");
                  new_activity_state = activity_state;
                }

              TRACE_MSG("new_activity_state = {}", new_activity_state);
            }

          TRACE_MSG("activity_state = {}", new_activity_state);
        }
    }

  // Start or stop timer.
  if (timer_enabled)
    {
      if (new_activity_state == ACTIVITY_ACTIVE && timer_state != STATE_RUNNING)
        {
          start_timer();
        }
      else if (new_activity_state != ACTIVITY_ACTIVE && timer_state == STATE_RUNNING)
        {
          stop_timer();
        }
    }

  activity_state = new_activity_state;
  TRACE_MSG("time, next limit, limit: {} {} {} {}",
            current_time,
            next_limit_time,
            limit_interval,
            (next_limit_time - current_time));

  TRACE_MSG("activity_state = {}", activity_state);

  if (autoreset_interval_predicate && next_pred_reset_time != 0 && current_time >= next_pred_reset_time)
    {
      // A next reset time was set and the current time >= reset time.
      // So reset the timer and send a reset event.
      reset_timer();

      last_pred_reset_time = TimeSource::get_real_time_sec_sync();
      next_pred_reset_time = 0;

      compute_next_predicate_reset_time();
      info.event = TIMER_EVENT_RESET;

      if (!activity_sensitive)
        {
          activity_state = ACTIVITY_IDLE;
          TRACE_MSG("pred reset, setting state = IDLE");
          stop_timer();
        }
    }
  else if (next_limit_time != 0 && current_time >= next_limit_time)
    {
      // A next limit time was set and the current time >= limit time.
      next_limit_time = 0;
      last_limit_time = TimeSource::get_real_time_sec_sync();
      last_limit_elapsed = get_elapsed_time();

      snooze_on_active = true;

      compute_next_limit_time();

      info.event = TIMER_EVENT_LIMIT_REACHED;
      // Its very unlikely (but not impossible) that this will overrule
      // the EventStarted. Hey, shit happens.

      if (!activity_sensitive)
        {
          activity_state = ACTIVITY_IDLE;
          TRACE_MSG("limit reached, setting state = IDLE");
        }
    }
  else if (next_reset_time != 0 && current_time >= next_reset_time)
    {
      // A next reset time was set and the current time >= reset time.

      next_reset_time = 0;

      bool natural = limit_enabled && limit_interval >= get_elapsed_time();

      reset_timer();

      info.event = natural ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_RESET;
      // Idem, may overrule the EventStopped.

      if (!activity_sensitive)
        {
          TRACE_MSG("reset reached, setting state = IDLE");
          activity_state = ACTIVITY_IDLE;
        }
    }
}

std::string
Timer::serialize_state() const
{
  stringstream ss;

  ss << timer_id << " " << TimeSource::get_real_time_sec_sync() << " " << get_elapsed_time() << " " << last_pred_reset_time << " "
     << total_overdue_time << " " << snooze_inhibited << " " << last_limit_time << " " << last_limit_elapsed << " "

#if defined(PLATFORM_OS_WINDOWS_NATIVE) // FIXME:
     << 0;
#else
     << timezone;
#endif

  return ss.str();
}

bool
Timer::deserialize_state(const std::string &state, int version)
{
  TRACE_ENTRY();
  istringstream ss(state);

  int64_t saveTime = 0;
  int64_t elapsed = 0;
  int64_t lastReset = 0;
  int64_t overdue = 0;
  int64_t now = TimeSource::get_real_time_sec_sync();
  int64_t llt = 0;
  int64_t lle = 0;
  int64_t tz = 0;
  bool si = false;

  ss >> saveTime >> elapsed >> lastReset >> overdue >> si >> llt >> lle;

  if (version == 3)
    {
      ss >> tz;
#if !defined(PLATFORM_OS_WINDOWS_NATIVE) // FIXME:
      tz -= timezone;
#endif
    }

  // Sanity check...
  if (lastReset > saveTime)
    {
      lastReset = saveTime;
    }

  // lastReset -= tz;

  TRACE_VAR(si, llt, lle);
  TRACE_VAR(snooze_inhibited);

  last_pred_reset_time = lastReset;
  total_overdue_time = overdue;
  elapsed_time = 0;
  last_start_time = 0;
  last_stop_time = 0;

  bool tooOld = ((autoreset_enabled && autoreset_interval != 0) && (now - saveTime > autoreset_interval));

  if (!tooOld)
    {
      if (autoreset_enabled)
        {
          next_reset_time = now + autoreset_interval;
        }
      elapsed_time = elapsed;
      snooze_inhibited = si;
    }

  // overdue, so snooze
  if (limit_enabled && get_elapsed_time() >= limit_interval)
    {
      last_limit_time = llt;
      last_limit_elapsed = lle;

      compute_next_limit_time();
    }

  compute_next_predicate_reset_time();

  TRACE_MSG("elapsed = {}", elapsed_time);
  return true;
}

void
Timer::set_state(int elapsed, int idle, int overdue)
{
  TRACE_ENTRY_PAR(elapsed, idle, overdue);

  elapsed_time = elapsed;
  elapsed_idle_time = idle;

  if (last_start_time != 0)
    {
      last_start_time = TimeSource::get_real_time_sec_sync();
    }

  if (last_stop_time != 0)
    {
      last_stop_time = TimeSource::get_real_time_sec_sync();
    }

  if (elapsed_idle_time > autoreset_interval && autoreset_enabled)
    {
      elapsed_idle_time = autoreset_interval;
    }

  if (overdue != -1)
    {
      total_overdue_time = overdue;
      if (limit_enabled && get_elapsed_time() > limit_interval)
        {
          total_overdue_time -= (get_elapsed_time() - limit_interval);
        }
    }

  compute_next_reset_time();
  compute_next_limit_time();
  compute_next_predicate_reset_time();
}

void
Timer::set_values(int64_t elapsed, int64_t idle)
{
  elapsed_time = elapsed;
  elapsed_idle_time = idle;

  last_start_time = 0;
  last_stop_time = 0;

  if (timer_state == STATE_RUNNING)
    {
      last_start_time = TimeSource::get_real_time_sec_sync();
    }
  else if (timer_state == STATE_STOPPED)
    {
      last_stop_time = TimeSource::get_real_time_sec_sync();
    }

  compute_next_limit_time();
  compute_next_reset_time();
  compute_next_predicate_reset_time();
}

void
Timer::set_state_data(const TimerStateData &data)
{
  int64_t time_diff = TimeSource::get_real_time_sec_sync() - data.current_time;

  elapsed_time = data.elapsed_time;
  elapsed_idle_time = data.elapsed_idle_time;
  last_pred_reset_time = data.last_pred_reset_time;
  total_overdue_time = data.total_overdue_time;

  last_limit_time = data.last_limit_time;
  last_limit_elapsed = data.last_limit_elapsed;
  snooze_inhibited = data.snooze_inhibited;

  if (last_pred_reset_time > 0)
    {
      last_pred_reset_time += time_diff;
    }

  if (last_limit_time > 0)
    {
      last_limit_time += time_diff;
    }

  last_start_time = 0;
  last_stop_time = 0;

  if (timer_state == STATE_RUNNING)
    {
      last_start_time = TimeSource::get_real_time_sec_sync();
    }
  else if (timer_state == STATE_STOPPED)
    {
      last_stop_time = TimeSource::get_real_time_sec_sync();
    }

  compute_next_limit_time();
  compute_next_reset_time();
  compute_next_predicate_reset_time();
}

void
Timer::get_state_data(TimerStateData &data)
{
  TRACE_ENTRY();
  data.current_time = TimeSource::get_real_time_sec_sync();

  data.elapsed_time = get_elapsed_time();
  data.elapsed_idle_time = get_elapsed_idle_time();
  data.last_pred_reset_time = last_pred_reset_time;
  data.total_overdue_time = total_overdue_time;

  data.last_limit_time = last_limit_time;
  data.last_limit_elapsed = last_limit_elapsed;
  data.snooze_inhibited = snooze_inhibited;

  TRACE_MSG("elapsed = {}", data.elapsed_time);
}

bool
Timer::is_running() const
{
  return timer_state == STATE_RUNNING;
}
