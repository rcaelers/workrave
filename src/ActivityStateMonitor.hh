// ActivityStateMonitor.hh --- ActivityStateMonitor for X11
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-21 20:01:21 robc>
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

#ifndef ACTIVITYSTATEMONITOR_HH
#define ACTIVITYSTATEMONITOR_HH

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

#include "Mutex.hh"

class ActivityMonitor;

#include "ActivityMonitorInterface.hh"
#include "InputMonitorInterface.hh"
#include "InputMonitorListenerInterface.hh"

//! The Activity State Monitor
class ActivityStateMonitor :
  public InputMonitorListenerInterface
{
public:
  //! Constructor.
  ActivityStateMonitor();

  //! Constructor.
  ActivityStateMonitor(int noise, int activity, int idle);

  //! Destructor.
  virtual ~ActivityStateMonitor();

  //! Suspends the activity monitoring.
  virtual void suspend();

  //! Resumes the activity monitoring.
  virtual void resume();
    
  //! Some activity is reported by the activity monitor.
  void action_notify();

  //! Mouse activity is reported by the activity monitor.
  void mouse_notify(int x, int y, int wheel = 0);

  //! Mouse button activity is reported by the activity monitor.
  void button_notify(int button_mask);

  //! Keyboard activity is reported by the activity monitor.
  void keyboard_notify(int key_code, int modifier);
  
  //! Terminate the monitor.
  void terminate();

  //! Force state te be idle.
  void force_idle();

  //! Return the current state
  ActivityState get_current_state();

  //! Sets the operation parameters.
  void set_parameters(int noise, int activity, int idle);

  //! Sets the operation parameters.
  void get_parameters(int &noise, int &activity, int &idle);

  //! Returns the total mouse movement
  int get_mouse_movement() const;

  //! Returns the total mouse click movement
  int get_mouse_click_movement() const;
  
private:
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

  // Statistical info.

  //! Total mouse movement;
  int total_movement;

  //! Total mouse movement bewteen click-point;
  int total_click_movement;
};

#endif // ACTIVITYSTATEMONITOR_HH
