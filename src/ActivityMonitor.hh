// ActivityMonitor.hh --- ActivityMonitor functionality
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#include <list>

#include "ActivityMonitorInterface.hh"

class InputMonitorInterface;
class ActivityListener;
class ActivityStateMonitor;
class Thread;

class ActivityMonitor :
  public ActivityMonitorInterface
{
public:
  ActivityMonitor(char *display);
  virtual ~ActivityMonitor();

  void start();
  void terminate();
  void suspend();
  void resume();
  void force_idle();
  void set_parameters(int noise, int activity, int idle);
  void get_parameters(int &noise, int &activity, int &idle);
  ActivityState get_current_state() const;

  void get_statistics(ActivityMonitorStatistics &stats) const;
  void set_statistics(const ActivityMonitorStatistics &stats);
  
private:
  //! The actual monitoring driver.
  InputMonitorInterface *input_monitor;
  
  //! The state monitor.
  ActivityStateMonitor *activity_state;

  // lazy me.
  friend class ActivityStateMonitor;
};

#endif // ACTIVITYMONITOR_HH
