// ActivityMonitorInterface.hh --- Interface definition for the Activity Monitor
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-02-22 09:43:06 robc>
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

#ifndef ACTIVITYMONITORINTERFACE_HH
#define ACTIVITYMONITORINTERFACE_HH

class ActivityListener;

enum ActivityState { ACTIVITY_UNKNOWN, ACTIVITY_SUSPENDED, ACTIVITY_IDLE, ACTIVITY_NOISE, ACTIVITY_ACTIVE } ;

class ActivityMonitorStatistics
{
public:
  //! Total mouse movement;
  int total_movement;

  //! Total mouse movement bewteen click-point;
  int total_click_movement;

  //! Total mouse movement time.
  int total_movement_time;

  //! Total mouse button clicks.
  int total_clicks;

  //! Total number of keystokes
  int total_keystrokes;
};


//! Interface that all activity monitors must support.
class ActivityMonitorInterface
{
public:
  //! Stops the activity monitoring.
  virtual void terminate() = 0;

  //! Suspends the activity monitoring.
  virtual void suspend() = 0;

  //! Resumes the activity monitoring.
  virtual void resume() = 0;

  //! Returns the current state
  virtual ActivityState get_current_state() const = 0;

  //! Force state to be idle.
  virtual void force_idle() = 0;

  //! Retrieves the statistics.
  virtual void get_statistics(ActivityMonitorStatistics &stats) const = 0;

  //! Retrieves the statistics.
  virtual void set_statistics(const ActivityMonitorStatistics &stats) = 0;

  //! Resets the statistics.
  virtual void reset_statistics() = 0;
  
};

#endif // ACTIVITYMONITORINTERFACE_HH
