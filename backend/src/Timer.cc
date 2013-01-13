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

#include "TimePred.hh"
#include "timeutil.h"

using namespace std;
using namespace workrave::utils;

#ifdef HAVE_EXTERN_TIMEZONE
#  ifndef HAVE_EXTERN_TIMEZONE_DEFINED
extern long timezone;
#  endif
#else
static int timezone = 0;
#endif

Timer::Ptr
Timer::create()
{
  return Ptr(new Timer());
}


//! Constructs a new break timer.
Timer::Timer() :
  timer_enabled(false),
  timer_frozen(false),
  activity_state(ACTIVITY_IDLE),
  timer_state(STATE_INVALID),
  snooze_interval(60),
  snooze_inhibited(false),
  limit_enabled(true),
  limit_interval(600),
  autoreset_enabled(true),
  autoreset_interval(120),
  autoreset_interval_predicate(NULL),
  elapsed_timespan(0),
  elapsed_timespan_at_last_limit(0),
  elapsed_idle_timespan(0),
  total_overdue_timespan(0),
  last_start_time(0),
  last_stop_time(0),
  last_reset_time(0),
  last_pred_reset_time(0),
  next_reset_time(0),
  next_pred_reset_time(0),
  next_limit_time(0),
  activity_sensitive(true),
  insensitive_mode(INSENSITIVE_MODE_IDLE_ON_LIMIT_REACHED)
{
}


//! Destructor
Timer::~Timer()
{
  delete autoreset_interval_predicate;
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

      compute_next_predicate_reset_time();
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
      // recompute.
      next_limit_time = 0;
      elapsed_timespan_at_last_limit = get_elapsed_time();
      compute_next_limit_time();

      if (!activity_sensitive)
        {
          // Start the clock in case of insensitive timer.
          activity_state = ACTIVITY_ACTIVE;
        }
    }
  
  TRACE_EXIT();
}


//! Prevents 'limit reached' snoozing until timer reset.
void
Timer::inhibit_snooze()
{
  TRACE_ENTER_MSG("Timer::inhibit_snooze", timer_id);
  snooze_inhibited = true;
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
          last_start_time = TimeSource::get_monotonic_time_sync();
          elapsed_idle_timespan = 0;
        }
      else
        {
          TRACE_MSG("Timer is frozen");
          // The timer is frozen, so we don't start counting 'active' time.
          // Instead, update the elapsed idle time.
          if (last_stop_time != 0)
            {
              elapsed_idle_timespan += (TimeSource::get_monotonic_time_sync() - last_stop_time);
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
  TRACE_ENTER_MSG("Timer::stop_timer", timer_id << " " << timer_state);

  if (timer_state != STATE_STOPPED)
    {
      // Update last stop time.
      last_stop_time = TimeSource::get_monotonic_time_sync();

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

      // When to re-send a limit reached?
      // (in case of !snooze_on_active)
      // compute_next_limit_time(); // TODO: if is really not needed
    }
  TRACE_EXIT();
}


//! Resets and stops the timer.
void
Timer::reset_timer()
{
  TRACE_ENTER_MSG("Timer::reset", timer_id << " " << timer_state);

  // Update total overdue.
  gint64 elapsed = get_elapsed_time();
  if (limit_enabled && elapsed > limit_interval)
    {
      total_overdue_timespan += (elapsed - limit_interval);
    }

  // Full reset.
  elapsed_timespan = 0;
  elapsed_timespan_at_last_limit = 0;
  last_reset_time = TimeSource::get_monotonic_time_sync();
  snooze_inhibited = false;

  if (timer_state == STATE_RUNNING)
    {
      // The timer is reset while running, Pretend the timer just started.
      last_start_time = TimeSource::get_monotonic_time_sync();
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
          last_stop_time = TimeSource::get_monotonic_time_sync();
        }
    }

  next_pred_reset_time = 0;
  compute_next_predicate_reset_time();
  TRACE_EXIT();
}


//! Freezes the elapsed time of the timer.
void
Timer::freeze_timer(bool freeze)
{
  TRACE_ENTER_MSG("Timer::freeze_timer",
                  timer_id << freeze << " " <<
                  timer_enabled << " " << activity_sensitive);

  // Only enabled activity sensitive timers care about freezing.
  if (timer_enabled && activity_sensitive)
    {
      if (freeze && !timer_frozen)
        {
          // freeze timer.
          if (last_start_time != 0 && timer_state == STATE_RUNNING)
            {
              elapsed_timespan += (TimeSource::get_monotonic_time_sync() - last_start_time);
              last_start_time = 0;
            }
        }
      else if (!freeze && timer_frozen)
        {
          // defrost timer.
          if (timer_state == STATE_RUNNING)
            {
              last_start_time = TimeSource::get_monotonic_time_sync();
              elapsed_idle_timespan = 0;

              compute_next_limit_time();
            }
        }
    }

  //test fix for Bug 746 -  Micro-break not counting down
  if (timer_enabled && !freeze && timer_frozen && timer_state == STATE_RUNNING && !last_start_time && !activity_sensitive)
    {
      last_start_time = TimeSource::get_monotonic_time_sync();
      elapsed_idle_timespan = 0;
      compute_next_limit_time();
    }

  timer_frozen = freeze;
  TRACE_EXIT();
}


//! Forces a activity insensitive timer to become idle
// void
// Timer::force_idle()
// {
//   TRACE_ENTER("Timer::force_idle");
//   if (!activity_sensitive)
//     {
//       TRACE_MSG("Forcing idle");
//       activity_state = ACTIVITY_IDLE;
//     }
//   TRACE_EXIT();
// }


//! Forces a activity insensitive timer to become active
void
Timer::restart_insensitive_timer()
{
  if (!activity_sensitive)
    {
      activity_state = ACTIVITY_ACTIVE;
    }
}


//! Perform timer processing.
TimerEvent
Timer::process(ActivityState new_activity_state)
{
  TRACE_ENTER_MSG("Timer::Process", timer_id << timer_id << " " << new_activity_state);

  // msvc can't handle std::string conditional tracepoints. use TRACE as the conditional
  bool TRACE = ( timer_id == "micro_pause" || timer_id == "rest_break" );
  (void) TRACE;

  gint64 current_time= TimeSource::get_monotonic_time_sync();

  // Default event to return.
  TimerEvent event = TIMER_EVENT_NONE;

  TRACE_MSG("enabled = " << timer_enabled);
  TRACE_MSG("last_start_time " << last_start_time);
  TRACE_MSG("next_pred_reset_time " << next_pred_reset_time);
  TRACE_MSG("next_reset_time " << next_reset_time);
  TRACE_MSG("time " << current_time);

  if (activity_sensitive)
    {
      // This timer responds to the activity monitoring.
      TRACE_MSG("is activity sensitive");
    }
  else
    {
      // This timer is activity insensitive. It periodically switches between
      // idle and active.
      TRACE_MSG("is not activity sensitive");
      TRACE_MSG("as = "   << activity_state <<
                " nas = " << new_activity_state <<
                " el ="   << get_elapsed_time());
      
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
              new_activity_state = activity_state;
            }
          
          TRACE_MSG("new state2 = " << activity_state << " " << new_activity_state);
        }

      if (activity_state == ACTIVITY_FORCED_IDLE)
        {
          new_activity_state = ACTIVITY_IDLE;
        }
      
      TRACE_MSG("state = " << new_activity_state);
      TRACE_MSG("time, next limit "
                << current_time << " "
                << next_limit_time << " "
                << limit_interval << " "
                << (next_limit_time - current_time)
                );
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
          TRACE_MSG("time, next limit "
                    << current_time << " "
                    << next_limit_time << " "
                    << limit_interval << " "
                    << (next_limit_time - current_time)
                    );
  TRACE_MSG("activity_state = " << activity_state);

  if (autoreset_interval_predicate &&
      next_pred_reset_time != 0 && TimeSource::get_real_time_sync() >= next_pred_reset_time)
    {
      // A next reset time was set and the current time >= reset time.
      // So reset the timer and send a reset event.
      reset_timer();

      last_pred_reset_time = TimeSource::get_real_time_sync();
      next_pred_reset_time = 0;

      compute_next_predicate_reset_time();
      event = TIMER_EVENT_RESET;

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
      elapsed_timespan_at_last_limit = get_elapsed_time();

      compute_next_limit_time();

      event = TIMER_EVENT_LIMIT_REACHED;
      // Its very unlikely (but not impossible) that this will overrule
      // the EventStarted. Hey, shit happends.

      if (!activity_sensitive)
        {
          activity_state = ACTIVITY_IDLE;
          TRACE_MSG("limit reached, setting state = IDLE");
        }
    }
  else if (next_reset_time != 0 && current_time >=  next_reset_time)
    {
      // A next reset time was set and the current time >= reset time.

      next_reset_time = 0;

      bool natural = limit_enabled && limit_interval >= get_elapsed_time();

      reset_timer();

      event = natural ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_RESET;
      // Idem, may overrule the EventStopped.

      if (!activity_sensitive)
        {
          TRACE_MSG("reset reached, setting state = IDLE");
          activity_state = ACTIVITY_IDLE;
        }
    }
  TRACE_EXIT();
  return event;
}


//! Returns the elapsed time.
gint64
Timer::get_elapsed_time() const
{
  gint64 ret = elapsed_timespan;

  if (timer_enabled && last_start_time != 0)
    {
      ret += (TimeSource::get_monotonic_time_sync() - last_start_time);
    }

  return ret;
}


//! Returns the elapsed idle time.
gint64
Timer::get_elapsed_idle_time() const
{
  gint64 ret = elapsed_idle_timespan;

  if (timer_enabled && last_stop_time != 0)
    {
      ret += (TimeSource::get_monotonic_time_sync() - last_stop_time);
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
Timer::set_auto_reset(TimePred *predicate)
{
  delete autoreset_interval_predicate;
  autoreset_interval_predicate = predicate;
  compute_next_predicate_reset_time();
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
gint64
Timer::get_auto_reset() const
{
  return autoreset_interval;
}


//! Returns the time the timer will reset.
gint64
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
gint64
Timer::get_limit() const
{
  return limit_interval;
}


//! Returns the time the limit will be reached.
gint64
Timer::get_next_limit_time() const
{
  return next_limit_time;
}




//! Sets the snooze interval of the timer.
void
Timer::set_snooze(gint64 t)
{
  snooze_interval = t;
}


//! Returns the snooze interval.
gint64
Timer::get_snooze() const
{
  return snooze_interval;
}




//! Marks timer sensitive or insensitive for activity
void
Timer::set_activity_sensitive(bool a)
{
  TRACE_ENTER_MSG("Timer::set_activity_sensitive", a);

  activity_sensitive = a;
  activity_state = ACTIVITY_IDLE;

  if (!activity_sensitive)
    {
      // If timer is made insensitive, start it if
      // it has some elasped time. Otherwise a daily limit
      // will never start (well, not until it resets...)
      gint64 elasped = get_elapsed_time();
      if (elasped > 0 && (elasped < limit_interval || !limit_enabled))
        {
          activity_state = ACTIVITY_ACTIVE;
        }
    }

  TRACE_EXIT();
}


//! Is this timer activity sensitive?
bool
Timer::is_activity_sensitive()
{
  return activity_sensitive;
}


//! Sets the activity insensitive mode
void
Timer::set_insensitive_mode(InsensitiveMode mode)
{
  insensitive_mode = mode;
}


//! Sets ID of this timer.
void
Timer::set_id(std::string id)
{
  timer_id = id;
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
     << TimeSource::get_real_time_sync() << " "
     << get_elapsed_time() << " "
     << last_pred_reset_time << " "
     << total_overdue_timespan << " "
     << snooze_inhibited << " "
     << 0 << " "
     << elapsed_timespan_at_last_limit << " "
     << timezone;

  return ss.str();
}


bool
Timer::deserialize_state(const std::string &state, int version)
{
  TRACE_ENTER("Timer::deserialize_state");
  istringstream ss(state);

  gint64 save_time = 0;
  gint64 elapsed = 0;
  gint64 last_reset = 0;
  gint64 overdue = 0;
  gint64 llt = 0;
  gint64 lle = 0;
  gint64 tz = 0;
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
      ss >> tz;
      tz -= timezone;
    }

  // Sanity check...
  if (last_reset > save_time)
    {
      last_reset = save_time;
    }

  // last_reset -= tz;

  TRACE_MSG(si << " " << llt << " " << lle);
  TRACE_MSG(snooze_inhibited);

  last_pred_reset_time = last_reset;
  total_overdue_timespan = overdue;
  elapsed_timespan = 0;
  last_start_time = 0;
  last_stop_time = 0;

  bool tooOld = ((autoreset_enabled && autoreset_interval != 0) && (TimeSource::get_real_time_sync() - save_time >  autoreset_interval));

  if (! tooOld)
    {
      if (autoreset_enabled)
        {
          next_reset_time = TimeSource::get_monotonic_time_sync() + autoreset_interval;
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

  compute_next_predicate_reset_time();

  TRACE_MSG("elapsed = " << elapsed_timespan);
  return true;
}

void
Timer::set_state(int elapsed, int idle, int overdue)
{
  TRACE_ENTER_MSG("Timer::set_state", elapsed << " " << idle << " " << overdue);

  elapsed_timespan = elapsed;
  elapsed_idle_timespan = idle;

  if (last_start_time != 0)
    {
      last_start_time = TimeSource::get_monotonic_time_sync();
    }

  if (last_stop_time != 0)
    {
      last_stop_time = TimeSource::get_monotonic_time_sync();
    }

  if (elapsed_idle_timespan > autoreset_interval && autoreset_enabled)
    {
      elapsed_idle_timespan = autoreset_interval;
    }

  if (overdue != -1)
    {
      total_overdue_timespan = overdue;
      if (limit_enabled && get_elapsed_time() > limit_interval)
        {
          total_overdue_timespan -= (get_elapsed_time() - limit_interval);
        }
    }

  compute_next_reset_time();
  compute_next_limit_time();
  compute_next_predicate_reset_time();

  TRACE_EXIT();
}


//! Returns the total overdue time of the timer.
gint64
Timer::get_total_overdue_time() const
{
  gint64 ret = total_overdue_timespan;
  gint64 elapsed = get_elapsed_time();

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
          last_pred_reset_time = TimeSource::get_real_time_sync();
        }

      next_pred_reset_time = autoreset_interval_predicate->get_next(last_pred_reset_time);
    }
}
