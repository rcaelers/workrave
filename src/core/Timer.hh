// Timer.hh --- Break Timer
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
// $Id$
//

#ifndef TIMER_HH
#define TIMER_HH

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

#include <string>
#include <list>

#include "TimerInterface.hh"
#include "ActivityMonitorInterface.hh"

using namespace std;

class TimeSource;
class TimePred;
class DataNode;

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
  bool enabled;
  
  TimerEvent event;
  time_t idle_time;
  time_t elapsed_time;
};


//! The Timer class.
/*!
 *  The Timer receives 'active' and 'idle' events from an activity monitor.
 *  Depending on settings, the Timer will start or stop the timer on these
 *  events.
 *
 */
class Timer :
  public TimerInterface
{
public:
  struct TimerStateData
  {
    time_t current_time;
    time_t elapsed_time;
    time_t elapsed_idle_time;
    time_t last_pred_reset_time;
    time_t total_overdue_time;
    time_t last_limit_time;
    time_t last_limit_elapsed;
    time_t snooze_inhibited;
  };
  
public:
  // Construction/Destruction.
  Timer(TimeSource *timeSource);
  virtual ~Timer();

  // Control
  void enable();
  void disable();
  void snooze_timer();
  void freeze_timer(bool f);

  // Timer processing.
  void process(ActivityState activityState, TimerInfo &info);
 
  // State inquiry
  time_t get_elapsed_time() const;
  time_t get_elapsed_idle_time() const;
  TimerState get_state() const;
  bool is_enabled() const;

  // Auto-resetting.
  void set_auto_reset(long t);
  void set_auto_reset(TimePred *predicate); 
  void set_auto_reset(string predicate);
  void set_auto_reset_enabled(bool b);
  bool is_auto_reset_enabled() const;
  time_t get_auto_reset() const;
  TimePred *get_auto_reset_predicate() const;
  time_t get_next_reset_time() const;
  
  // Limiting.
  void set_limit(long t);
  void set_limit_enabled(bool b);
  bool is_limit_enabled() const;
  time_t get_limit() const;
  time_t get_next_limit_time() const;
    
  // Activity/Idle couting.
  void set_for_activity(bool a);
  bool is_activity_timer() const;

  time_t get_snooze() const;
  
  // Timer ID
  void set_id(string id);
  string get_id() const;

  string serialize_state() const;
  bool deserialize_state(string state);

  void set_state_data(const TimerStateData &data);
  void get_state_data(TimerStateData &data);
  void set_values(int elapsed, int idle);
  
  // Misc
  void set_snooze_interval(time_t time);
  void inhibit_snooze();
  void set_activity_monitor(ActivityMonitorInterface *am);
  bool has_activity_monitor() const;

  time_t get_total_overdue_time() const;
  void daily_reset_timer();

  void shift_time(int delta);

  void start_timer();
  void stop_timer();
  void reset_timer();

  
private:
  //! Activity or Idle timer.
  /*! If \c true this timer counts 'active' time. I.e. the timer is started on
   *  an activity-event and stopped on a idle-event. In case this bool is \c
   *  false, the opposite happens.
   */
  bool activity_timer;

  //! Is this timer enabled ?
  bool timer_enabled;

  //! Is the timer frozen? A frozen timer only counts idle time.
  bool timer_frozen;
  
  //! State of the state monitor.
  ActivityState activity_state;

  //! State of the timer.
  TimerState timer_state;

  //! The previous state of the timer. (i.e. before process() was called).
  TimerState previous_timer_state;

  //! Default snooze time
  time_t snooze_interval;

  //! Snooze on active time instead of on actual time.
  bool snooze_on_active;
  
  //! Don't snooze til next reset or changes.
  bool snooze_inhibited;

  //! Is the timer limit enabled?
  bool limit_enabled;
  
  //! Timer limit interval.
  time_t limit_interval;

  //! Is the timer auto reset enabled?
  bool autoreset_enabled;

  //! Automatic reset time interval.
  time_t autoreset_interval;

  //! Auto reset time predicate. (or NULL if not used)
  TimePred *autoreset_interval_predicate;

  //! Is the state of this timer saved on exit and restored on startup ?
  bool restore_enabled;
  
  //! Elapsed time.
  time_t elapsed_time;

  //! Elapsed Idle time.
  time_t elapsed_idle_time;

  //! Last time the limit was reached.
  time_t last_limit_time;

  //! The total elapsed time the last time the limit was reached.
  time_t last_limit_elapsed;
  
  //! Time when the timer was last started.
  time_t last_start_time;
  
  //! Time when the timer was last reset.
  time_t last_reset_time;

  //! Time when the timer was last stopped.
  time_t last_stop_time;

  //! Next automatic reset time.
  time_t next_reset_time;

  //! Time when the timer was last reset because of a predicate.
  time_t last_pred_reset_time;

  //! Next automatic predicate reset time.
  time_t next_pred_reset_time;

  //! Next limit time.
  time_t next_limit_time;

  //! Total overdue time.
  time_t total_overdue_time;
  
  //! Id of the timer.
  string timer_id;

  //! Our source of time.
  TimeSource *time_source;

  //! Activity Mobnitor to use.
  ActivityMonitorInterface *activity_monitor;
  
private:
  void compute_next_limit_time();
  void compute_next_reset_time();
  void compute_next_predicate_reset_time();

  // Implementation of ActivityListener.
  void activity_notify();
  void idle_notify();
};

#include "Timer.icc"

#endif // TIMER_HH
