// TimerInterface.hh --- The Break Timer Query Interface
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-16 15:10:12 caelersr>
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
// $Id$
//

#ifndef TIMERINTERFACE_HH
#define TIMERINTERFACE_HH

class TimeSource;
class TimePred;

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

class TimePred;

enum TimerEvent
  {
    TIMER_EVENT_NONE,
    TIMER_EVENT_STOPPED,
    TIMER_EVENT_STARTED,
    TIMER_EVENT_RESET,
    TIMER_EVENT_NATURAL_RESET,
    TIMER_EVENT_LIMIT_REACHED,
  };

struct TimerInfo
{
  TimerEvent event;
  time_t idle_time;
  time_t elapsed_time;
};


//! The Timer interface.
/*!
 */
class TimerInterface
{
public:
  enum TimerState
    {
      STATE_INVALID,
      STATE_RUNNING,
      STATE_STOPPED
    };
  
public:
  virtual time_t get_elapsed_time() const = 0;
  virtual time_t get_elapsed_idle_time() const = 0;
  virtual TimerState get_state() const = 0;
  virtual bool is_enabled() const = 0;

  // Auto-resetting.
  virtual time_t get_auto_reset() const = 0;
  virtual TimePred *get_auto_reset_predicate() const =0;
  virtual time_t get_next_reset_time() const = 0;
  virtual bool is_auto_reset_enabled() const = 0;
  
  // Limiting.
  virtual time_t get_limit() const = 0;
  virtual time_t get_next_limit_time() const = 0;
  virtual bool is_limit_enabled() const = 0;

  // Snoozing.
  virtual time_t get_snooze() const = 0;
  
  // Activity/Idle counting.
  virtual bool is_activity_timer() const = 0;

  // Control
  virtual void snooze_timer() = 0;
  virtual void stop_timer() = 0;
  virtual void reset_timer() = 0;
  
  // Timer ID
  virtual string get_id() const = 0;
};

#endif // TIMERINTERFACE_HH
