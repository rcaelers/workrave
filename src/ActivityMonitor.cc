// ActivityMonitor.cc --- ActivityMonitor
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
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

#include "ActivityMonitor.hh"
#include "ActivityMonitorListenerInterface.hh"

#include "debug.hh"
#include "timeutil.h"
#include <assert.h>
#include <math.h>

#include <stdio.h>
#include <sys/types.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(HAVE_X)
#include "X11InputMonitor.hh"
#elif defined(WIN32)
#include "Win32InputMonitor.hh"
#endif


//! Constructor.
ActivityMonitor::ActivityMonitor(char *display) :
  activity_state(ACTIVITY_IDLE),
  prev_x(-1),
  prev_y(-1),
  click_x(-1),
  click_y(-1),
  listener(NULL)
{
  TRACE_ENTER("ActivityMonitor::ActivityMonitor");

  first_action_time.tv_sec = 0;
  first_action_time.tv_usec = 0;

  last_action_time.tv_sec = 0;
  last_action_time.tv_usec = 0;
  
  noise_threshold.tv_sec = 1;
  noise_threshold.tv_usec = 0;

  activity_threshold.tv_sec = 2;
  activity_threshold.tv_usec = 0;

  idle_threshold.tv_sec = 5;
  idle_threshold.tv_usec = 0;

  last_mouse_time.tv_sec = 0;
  last_mouse_time.tv_usec = 0;
  
  total_mouse_time.tv_sec = 0;
  total_mouse_time.tv_usec = 0;

  statistics.total_movement = 0;
  statistics.total_click_movement = 0;
  statistics.total_clicks = 0;
  statistics.total_keystrokes = 0;
  
#if defined(HAVE_X)
  input_monitor = new X11InputMonitor(display);
#elif defined(WIN32)
  input_monitor = new Win32InputMonitor();
#endif

  input_monitor->init(this);

  TRACE_EXIT();
}


//! Destructor.
ActivityMonitor::~ActivityMonitor()
{
  TRACE_ENTER("ActivityMonitor::~ActivityMonitor");

  if (input_monitor != NULL)
    {
      delete input_monitor;
    }

  TRACE_EXIT();
}
 

void
ActivityMonitor::terminate()
{
  TRACE_ENTER("ActivityMonitor::terminate");

  input_monitor->terminate();

  TRACE_EXIT();
}



void
ActivityMonitor::set_parameters(int noise, int activity, int idle)
{
  noise_threshold.tv_sec = noise / 1000;
  noise_threshold.tv_usec = (noise % 1000) * 1000;

  activity_threshold.tv_sec = activity / 1000;
  activity_threshold.tv_usec = (activity % 1000) * 1000;

  idle_threshold.tv_sec = idle / 1000;
  idle_threshold.tv_usec = (idle % 1000) * 1000;

  // The easy way out.
  activity_state = ACTIVITY_IDLE;
}


void
ActivityMonitor::get_parameters(int &noise, int &activity, int &idle)
{
  noise = noise_threshold.tv_sec * 1000 + noise_threshold.tv_usec / 1000;
  activity = activity_threshold.tv_sec * 1000 + activity_threshold.tv_usec / 1000;
  idle = idle_threshold.tv_sec * 1000 + idle_threshold.tv_usec / 1000;
}


void
ActivityMonitor::mouse_notify(int x, int y, int wheel_delta)
{
  static const int sensitivity = 3;

  const int delta_x = x - prev_x;
  const int delta_y = y - prev_y;
  prev_x = x;
  prev_y = y;
  
  if (abs(delta_x) >= sensitivity || abs(delta_y) >= sensitivity
      || wheel_delta != 0)
    {
      statistics.total_movement += int(sqrt((double)(delta_x * delta_x + delta_y * delta_y)));
      
      action_notify();

      struct timeval now, tv;

      gettimeofday(&now, NULL);
      tvSUBTIME(tv, now, last_mouse_time);
      
      if (!tvTIMEEQ0(last_mouse_time) && tv.tv_sec < 1)
        {
          tvADDTIME(total_mouse_time, total_mouse_time, tv)
        }

      last_mouse_time = now;
      
    }
  lock.unlock();
}


void
ActivityMonitor::button_notify(int button_mask, bool is_press)
{
  (void)button_mask;
  lock.lock();
  if (click_x != -1)
    {
      int delta_x = click_x - prev_x;
      int delta_y = click_y - prev_y;
     
      statistics.total_click_movement += int(sqrt((double)(delta_x * delta_x + delta_y * delta_y)));
    }
  
  click_x = prev_x;
  click_y = prev_y;
  action_notify();

  if (is_press)
    {
      statistics.total_clicks++;
    }
  
  lock.unlock();
}


void
ActivityMonitor::keyboard_notify(int key_code, int modifier)
{
  (void)key_code;
  (void)modifier;
  lock.lock();
  action_notify();
  statistics.total_keystrokes++;
  lock.unlock();
}


void
ActivityMonitor::action_notify()
{
  lock.lock();
  
  struct timeval now;
  gettimeofday(&now, NULL);

  switch (activity_state)
    {
    case ACTIVITY_IDLE:
      {
        first_action_time = now;
        last_action_time = now;

        if (tvTIMEEQ0(activity_threshold))
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
        struct timeval tv;
        
        tvSUBTIME(tv, now, last_action_time);
        if (tvTIMEGT(tv, noise_threshold))
          {
            first_action_time = now;
            //TRACE_MSG("Noise");
          }
        else
          {
            bool active = false;
#if 0
            tvADDTIME(tv, first_action_time, activity_threshold); //TODO: precompute
            tvSUBTIME(tv, tv, now);                                     
            active = tvTIMELT(tv, noise_threshold);
#else
            tvSUBTIME(tv, now, first_action_time);
            active = tvTIMEGEQ(tv, activity_threshold);
#endif                               
            if (active)
              {
                activity_state = ACTIVITY_ACTIVE;
              }
            else
              {
                // noise
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


//! Suspends the activity monitoring.
void
ActivityMonitor::suspend()
{
  lock.lock();
  activity_state = ACTIVITY_SUSPENDED;
  lock.unlock();
}


//! Resumes the activity monitoring.
void
ActivityMonitor::resume()
{
  lock.lock();
  activity_state = ACTIVITY_IDLE;
  lock.unlock();
}


//! Returns the current state.
ActivityState
ActivityMonitor::get_current_state()
{
  lock.lock();

  if (activity_state == ACTIVITY_ACTIVE)
    {
      struct timeval now, tv;
      gettimeofday(&now, NULL);

      tvSUBTIME(tv, now, last_action_time);
      if (tvTIMEGT(tv, idle_threshold))
        {
          activity_state = ACTIVITY_IDLE;
        }
    }

  lock.unlock();
  return activity_state;
}


void
ActivityMonitor::force_idle()
{
  lock.lock();
  activity_state = ACTIVITY_IDLE;
  lock.unlock();
}


void
ActivityMonitor::get_statistics(ActivityMonitorStatistics &stats) const
{
  stats = statistics;
  stats.total_movement_time = total_mouse_time.tv_sec;
}


void
ActivityMonitor::set_statistics(const ActivityMonitorStatistics &stats)
{
  statistics = stats;
  total_mouse_time.tv_sec = stats.total_movement_time;
  total_mouse_time.tv_usec = 0;
}


void
ActivityMonitor::reset_statistics()
{
  total_mouse_time.tv_sec = 0;
  total_mouse_time.tv_usec = 0;

  statistics.total_movement = 0;
  statistics.total_click_movement = 0;
  statistics.total_clicks = 0;
  statistics.total_keystrokes = 0;
}

void
ActivityMonitor::set_listener(ActivityMonitorListenerInterface *l)
{
  lock.lock();
  listener = l;
  lock.unlock();
}


void
ActivityMonitor::call_listener()
{
  ActivityMonitorListenerInterface *l = NULL;

  lock.lock();
  l = listener;
  lock.unlock();

  if (l != NULL)
    {
      if (!l->action_notify())
        {
          lock.lock();
          listener = NULL;
          lock.unlock();
        }
    }
}

void
ActivityMonitor::shift_time(int delta)
{
  struct timeval d;

  tvSETTIME(d, delta, 0)
    
  if (!tvTIMEEQ0(last_action_time))
    tvADDTIME(last_action_time, last_action_time, d);

  if (!tvTIMEEQ0(first_action_time))
    tvADDTIME(first_action_time, first_action_time, d);

  if (!tvTIMEEQ0(last_mouse_time))
    tvADDTIME(last_mouse_time, last_mouse_time, d);
}

