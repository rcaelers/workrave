// ActivityMonitor.hh --- ActivityMonitor functionality
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

#ifndef ACTIVITYMONITOR_HH
#define ACTIVITYMONITOR_HH

#include "ActivityMonitorInterface.hh"
#include "InputMonitorInterface.hh"
#include "InputMonitorListenerInterface.hh"
#include "Mutex.hh"

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

class ActivityListener;
class InputMonitorInterface;
class Thread;

class ActivityMonitor :
  public InputMonitorListenerInterface,
  public ActivityMonitorInterface
{
public:
  ActivityMonitor(char *display);
  virtual ~ActivityMonitor();

  //! Starts the monitor.
  void start();

  //! Terminates the monitor.
  void terminate();
  
  //! Suspends the activity monitoring.
  void suspend();

  //! Resumes the activity monitoring.
  void resume();

  //! Forces state te be idle.
  void force_idle();

  //! Returns the current state
  ActivityState get_current_state();

  //! Sets the operation parameters.
  void set_parameters(int noise, int activity, int idle);

  //! Sets the operation parameters.
  void get_parameters(int &noise, int &activity, int &idle);

  //! Returns the statistics.
  void get_statistics(ActivityMonitorStatistics &stats) const;

  //! Sets the statistics
  void set_statistics(const ActivityMonitorStatistics &stats);

  //! Resets the statistics.
  void reset_statistics();

  //!
  void set_listener(ActivityMonitorListenerInterface *l);
  
  //! Activity is reported by the input monitor.
  void action_notify();

  //! Mouse activity is reported by the input monitor.
  void mouse_notify(int x, int y, int wheel = 0);

  //! Mouse button activity is reported by the input monitor.
  void button_notify(int button_mask, bool is_press);

  //! Keyboard activity is reported by the input monitor.
  void keyboard_notify(int key_code, int modifier);

  void shift_time(int delta);
  
private:
  void call_listener();
  
private:
  //! The actual monitoring driver.
  InputMonitorInterface *input_monitor;

  //! the current state.
  ActivityState activity_state;

  //! Internal locking
  Mutex lock;

  //! Previous X coordinate
  int prev_x;
  
  //! Previous Y coordinate
  int prev_y;

  //! Previous X-click coordinate
  int click_x;
  
  //! Previous Y-click coordinate
  int click_y;

  //! Last time activity was detected
  struct timeval last_action_time;

  //! First time the \c ACTIVITY_IDLE state was left.
  struct timeval first_action_time;

  //! The noise threshold
  struct timeval noise_threshold;

  //! The activity threshold.
  struct timeval activity_threshold;

  //! The idle threshold.
  struct timeval idle_threshold;

  //! Statistical info.
  ActivityMonitorStatistics statistics;

  //! Last time a mouse event was received.
  struct timeval last_mouse_time;

  //! Last time a mouse event was received.
  struct timeval total_mouse_time;

  //!
  ActivityMonitorListenerInterface *listener;
};

#endif // ACTIVITYMONITOR_HH
