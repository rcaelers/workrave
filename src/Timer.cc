// Timer.cc --- break timer
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-02-23 11:26:25 robc>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <sstream>
#include <stdio.h>
#include <math.h>

#include "Timer.hh"

#include "TimePredFactory.hh"
#include "TimePred.hh"
#include "TimeSource.hh"

#include "timeutil.h"


//! Constructs a new break timer.
/*!
 *  \param timeSource the Timer wil obtain the current time from this source of
 *                    time.
 */
Timer::Timer(TimeSource *timeSource) :
  activity_timer(true),
  timer_enabled(false),
  timer_frozen(false),
  activity_state(ACTIVITY_UNKNOWN),
  timer_state(STATE_INVALID),
  previous_timer_state(STATE_INVALID),
  snooze_interval(60),
  snooze_on_active(true),
  snooze_inhibited(false),
  limit_enabled(true),
  limit_interval(600),
  autoreset_enabled(true),
  autoreset_interval(120),
  autoreset_interval_predicate(NULL),
  restore_enabled(true),
  elapsed_time(0),
  elapsed_idle_time(0),
  last_limit_time(0),
  last_limit_elapsed(0),
  last_start_time(0),
  last_reset_time(0),
  last_stop_time(0),
  next_reset_time(0),
  last_pred_reset_time(0),
  next_pred_reset_time(0),
  next_limit_time(0),
  total_overdue_time(0),
  time_source(timeSource),
  activity_monitor(NULL)
{
}


//! Destructor
Timer::~Timer()
{
  if (autoreset_interval_predicate != NULL)
    {
      delete autoreset_interval_predicate;
    }
}


//! Enable activity event monitoring.
void
Timer::enable()
{
  if (!timer_enabled)
    {
      timer_enabled = true;
      snooze_inhibited = false;
      snooze_on_active = true;
      stop_timer();

      // TODO: new, testing.
      if (autoreset_enabled && autoreset_interval != 0 && get_elapsed_time() == 0)
        {
          elapsed_idle_time = autoreset_interval;
        }
           
      if (get_elapsed_time() >= limit_interval)
        {
          // Break is overdue, force a snooze.
          last_limit_time = time_source->get_time();
          last_limit_elapsed = 0;
          compute_next_limit_time();
        }

      compute_next_predicate_reset_time();
    }
}


//! Disable activity event monitoring.
void
Timer::disable()
{
  if (timer_enabled)
    {
      timer_enabled = false;
      stop_timer();
    }
}


//! Set Limit time in seconds.
/*!
 * \param limitTime time at which this timer reaches its limit.
 *
 */
void
Timer::set_limit(long limitTime)
{
  limit_interval = limitTime;

  if (get_elapsed_time() < limitTime)
    {
      // limit increased, pretend there was no limit-reached yet.
      // FIXME: Check if the timer would have been overdue with new settings.
      last_limit_time = 0;
      last_limit_elapsed = 0;
    }

  compute_next_limit_time();
}


//! Enable/Disable limiting
/*!
 * \param b indicates whether the timer limit must be enabled (\c true) or
 *          disabled (\c false).
 */
void
Timer::set_limit_enabled(bool b)
{
  limit_enabled = b;
  compute_next_limit_time();
}


void
Timer::set_snooze_interval(time_t t)
{
  snooze_interval = t;
}


void
Timer::inhibit_snooze()
{
  snooze_inhibited = true;
}


//! Enable/Disable auto-reset
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


//! Set auto-reset time period.
/*!
 * \param resetTime after this amount of idle time the timer will reset itself.
 */
void
Timer::set_auto_reset(long resetTime)
{
  if (resetTime > autoreset_interval)
    {
      snooze_inhibited = false;
    }

  autoreset_interval = resetTime;
  compute_next_reset_time();
}


//! Set auto-reset predicate.
/*!
 * \param predicate auto-reset predicate.
 */
void
Timer::set_auto_reset(string predicate)
{
  autoreset_interval_predicate = TimePredFactory::create_time_pred(predicate);
  compute_next_predicate_reset_time();
}



//! Compute the time the limit will be reached.
void
Timer::compute_next_limit_time()
{
  // default action.
  next_limit_time = 0;

  if (timer_enabled && last_limit_time != 0 && !snooze_on_active)
    {
      // Timer already reached limit
      if (!snooze_inhibited)
        {
          next_limit_time = last_limit_time + snooze_interval;
        }
    }
  else if (timer_enabled && timer_state == STATE_RUNNING && last_start_time != 0 &&
           limit_enabled && limit_interval != 0)
    { 
      // We are enabled, running and a limit != 0 was set.
      // So update our current Limit.

      if (last_limit_time != 0)
        {
          // Limit already reached.
          if (snooze_on_active && !snooze_inhibited)
            {
              next_limit_time = last_start_time - elapsed_time + last_limit_elapsed + snooze_interval;
            }
        }
      else
        {
          // new limit = last start time + limit - elapsed.
          next_limit_time = last_start_time + limit_interval - elapsed_time;
        }
    }
}


//! Compute the time the auto-reset must take place.
void
Timer::compute_next_reset_time()
{
  if (timer_enabled && timer_state == STATE_STOPPED && last_stop_time != 0 &&
      autoreset_enabled && autoreset_interval != 0)
    {
      // We are enabled, not running and a limit != 0 was set.

      // next reset time = last stop time + auto reset
      next_reset_time = last_stop_time + autoreset_interval - elapsed_idle_time;

      if (next_reset_time <= last_reset_time)
        {
          next_reset_time = 0;
        }
    }
  else
    {
      // Just in case....
      next_reset_time = 0;
    }
}


void
Timer::compute_next_predicate_reset_time()
{
  if (timer_enabled && autoreset_interval_predicate)
    {
      if (last_pred_reset_time == 0)
        {
          last_pred_reset_time = time_source->get_time();
        }
        
      autoreset_interval_predicate->set_last(last_pred_reset_time);
      next_pred_reset_time = autoreset_interval_predicate->get_next();
    }
}

//! Daily Reset.
void
Timer::daily_reset_timer()
{
  total_overdue_time = 0;
}


//! Reset and stop the timer.
void
Timer::reset_timer()
{
  // Update total overdue.
  time_t elapsed = get_elapsed_time();
  if (elapsed > limit_interval)
    {
      total_overdue_time += (elapsed - limit_interval);
    }
  
  // Full reset.
  elapsed_time = 0;
  last_limit_time = 0;
  last_limit_elapsed = 0;
  last_reset_time = time_source->get_time();
  snooze_inhibited = false;
  snooze_on_active = true;
  
  if (timer_state == STATE_RUNNING)
    {
      // Pretend the timer just started.
      last_start_time = time_source->get_time();
      last_stop_time = 0;
      
      compute_next_limit_time();
      next_reset_time = 0;
      elapsed_idle_time = 0;
    }
  else
    {
      // Timer stopped.
      last_start_time = 0;
      next_reset_time = 0;
      next_limit_time = 0;

      // TODO: new:
      if (autoreset_enabled && autoreset_interval != 0)
        {
          elapsed_idle_time = autoreset_interval;
        }
    }
      
  next_pred_reset_time = 0;
  compute_next_predicate_reset_time();
}


//! Start the timer.
void
Timer::start_timer()
{
  if (timer_state != STATE_RUNNING)
    {
      // Set last start and stop times.
      if (!timer_frozen)
        {
          // Timer is not frozen, so let's start.
          last_start_time = time_source->get_time();
          elapsed_idle_time = 0;
        }
      else
        {
          // Update elapsed time.
          if (last_stop_time != 0)
            {
              elapsed_idle_time += (time_source->get_time() - last_stop_time);
            }
          last_start_time = 0;
        }
      
      last_stop_time = 0;
      next_reset_time = 0;
  
      // update state.
      timer_state = STATE_RUNNING;

      // When to generate a limit-reached-event.
      compute_next_limit_time();
    }
}


//! Stop the timer.
void
Timer::stop_timer()
{
  if (timer_state != STATE_STOPPED)
    {
      // Update last stop time.
      last_stop_time = time_source->get_time();

      // Update elapsed time.
      if (last_start_time != 0)
        {
          // But only if we are running...

          elapsed_time += (last_stop_time - last_start_time);
        }
  
      // Reset last start time.
      last_start_time = 0;
  
      // Update state.
      timer_state = STATE_STOPPED;

      // When to reset the timer.
      compute_next_reset_time();

      // 
      compute_next_limit_time();
    }
}


//! Snoozes the Timer.
/*!
 *  When the limit of this timer was reached, the Limit Reached event will be
 *  re-sent.
 */
void
Timer::snooze_timer()
{
  if (timer_enabled) //  && get_elapsed_time() >= limit_interval)
    {
      snooze_on_active = false;

      // recompute.
      last_limit_time = time_source->get_time();
      //last_limit_elapsed = get_elapsed_time();
      compute_next_limit_time();
    }
}


void
Timer::freeze_timer(bool freeze)
{
  if (timer_enabled)
    {
      if (freeze && !timer_frozen)
        {
          // FIXME: Why does this say "last_limit_time" ??? should it be last_start_time???
          if (last_limit_time != 0 && timer_state == STATE_RUNNING)
            {
              elapsed_time += (time_source->get_time() - last_start_time);
              last_start_time = 0;
            }
        }
      else if (!freeze && timer_frozen)
        {
          if (timer_state == STATE_RUNNING)
            {
              last_start_time = time_source->get_time();
              elapsed_idle_time = 0;
            }
        }
    }
  timer_frozen = freeze;
}


//! Returns the elapsed idle time.
time_t
Timer::get_elapsed_idle_time() const
{
  time_t ret = elapsed_idle_time;
  
  if (timer_enabled && last_stop_time != 0)
    {
      // We are not running.
      
      ret += (time_source->get_time() - last_stop_time);
    }

  return ret;
}


//! Returns the elapsed time.
time_t
Timer::get_elapsed_time() const
{
  time_t ret = elapsed_time;

  if (timer_enabled && last_start_time != 0)
    {
      // We are running:
      // total elasped = elapes + (last start time - current time)

      ret += (time_source->get_time() - last_start_time);
    }
  else
    {
      // We are not running
      // total elapsed = elapsed
    }

  return ret;
}


time_t
Timer::get_total_overdue_time() const
{
  time_t ret = total_overdue_time;
  time_t elapsed = get_elapsed_time();
  if (elapsed > limit_interval)
    {
      ret += (elapsed - limit_interval);
    }

  return ret;
}


//! Activity callback from activity monitor.
void
Timer::activity_notify()
{
  if (timer_enabled)
    {
      if (activity_timer)
        {
          start_timer();
        }
      else
        {
          stop_timer();
        }
    }
}


//! Idle callback from activity monitor.
void
Timer::idle_notify()
{
  if (timer_enabled)
    {
      if (activity_timer)
        {
          stop_timer();
        }
      else
        {
          start_timer();
        }
    }
}


//! Perform timer processing.
void
Timer::process(ActivityState activityState, TimerInfo &info)
{
  if (activity_monitor != NULL)
    {
      activityState = activity_monitor->get_current_state();
    }
        
  info.event = TIMER_EVENT_NONE;
  info.idle_time = get_elapsed_idle_time();
  info.elapsed_time = get_elapsed_time();
    
  if (activityState == ACTIVITY_ACTIVE && timer_state != STATE_RUNNING)
    {
      activity_notify();
    }
  else if (activityState != ACTIVITY_ACTIVE && timer_state == STATE_RUNNING)
    {
      idle_notify();
    }
  
  activity_state = activityState;
  
  time_t current_time= time_source->get_time();

  if (autoreset_interval_predicate && next_pred_reset_time != 0 && current_time >=  next_pred_reset_time)
    {
      // A next reset time was set and the current time >= reset time.

      reset_timer();

      last_pred_reset_time = time_source->get_time();
      next_pred_reset_time = 0;
      
      compute_next_predicate_reset_time();
      info.event = TIMER_EVENT_RESET;
    }
  else if (next_limit_time != 0 && current_time >=  next_limit_time)
    {
      // A next limit time was set and the current time >= limit time.
      next_limit_time = 0;
      last_limit_time = time_source->get_time();
      last_limit_elapsed = get_elapsed_time();

      snooze_on_active = true;

      compute_next_limit_time();
      
      info.event = TIMER_EVENT_LIMIT_REACHED;
      // Its very unlikely (but not impossible) that this will overrule
      // the EventStarted. Hey, shit happends.
    }
  else if (next_reset_time != 0 && current_time >=  next_reset_time)
    {
      // A next reset time was set and the current time >= reset time.
      
      next_reset_time = 0;
      
      bool natural = (limit_interval >=  get_elapsed_time());
      
      reset_timer();
      
      info.event = natural ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_RESET;
      // Idem, may overrule the EventStopped.
    }
  else
    {
      switch (timer_state)
        {
        case STATE_RUNNING:
          {
            if (previous_timer_state == STATE_STOPPED)
              {
                info.event = TIMER_EVENT_STARTED;
              }
          }
          break;
          
        case STATE_STOPPED:
          {
            if (previous_timer_state == STATE_RUNNING)
              {
                info.event = TIMER_EVENT_STOPPED;
              }
          }
          break;
          
        default:
          break;
        }
    }
  
  previous_timer_state = timer_state;
}


std::string
Timer::serialize_state() const
{
  stringstream ss;

  if (restore_enabled)
    {
      ss << timer_id << " " 
         << time_source->get_time() << " "
         << get_elapsed_time() << " "
         << last_pred_reset_time << " "
         << total_overdue_time;
    }
  
  return ss.str();
}


bool
Timer::deserialize_state(std::string state)
{
  istringstream ss(state);

  time_t saveTime = 0;
  time_t elapsed = 0;
  time_t lastReset = 0;
  time_t overdue = 0;
  time_t now = time_source->get_time();
  
  ss >> saveTime
     >> elapsed
     >> lastReset
     >> overdue;
  
  last_pred_reset_time = lastReset;
  total_overdue_time = overdue;
  elapsed_time = 0;

  bool tooOld = ((autoreset_enabled && autoreset_interval != 0) && (now - saveTime >  autoreset_interval));

  if (! tooOld)
    {  
      next_reset_time = now + autoreset_interval;
      elapsed_time = elapsed;
    }

  // overdue, so snooze
  if (get_elapsed_time() >= limit_interval)
    {
      last_limit_time = time_source->get_time();
      last_limit_elapsed = 0;
      
      compute_next_limit_time();
    }

  compute_next_predicate_reset_time();

  return true;
}


void
Timer::set_state_data(const TimerStateData &data)
{
  time_t time_diff =  time_source->get_time() - data.current_time;

  elapsed_time = data.elapsed_time;
  elapsed_idle_time = data.elapsed_idle_time;
  last_pred_reset_time = data.last_pred_reset_time + time_diff;
  total_overdue_time = data.total_overdue_time;
  
  timer_state = STATE_INVALID;
  previous_timer_state = STATE_INVALID;
  last_start_time = 0;
  last_stop_time = 0;

  compute_next_predicate_reset_time();
}


void
Timer::get_state_data(TimerStateData &data)
{
  TRACE_ENTER("Timer::get_state_data");
  data.current_time = time_source->get_time();
  
  data.elapsed_time = get_elapsed_time();
  data.elapsed_idle_time = get_elapsed_idle_time();
  data.last_pred_reset_time = last_pred_reset_time;
  data.total_overdue_time = total_overdue_time;

  TRACE_MSG("elapsed = " << data.elapsed_time);
  TRACE_EXIT();
}
