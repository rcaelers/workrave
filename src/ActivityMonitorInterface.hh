// ActivityMonitorInterface.hh --- Interface definition for the Activity Monitor
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-09-03 15:56:01 caelersr>
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
};

#endif // ACTIVITYMONITORINTERFACE_HH
