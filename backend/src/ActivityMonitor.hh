// ActivityMonitor.hh --- ActivityMonitor functionality
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

#ifndef ACTIVITYMONITOR_HH
#define ACTIVITYMONITOR_HH

#include "config/Config.hh"

using namespace workrave::config;

class IActivityMonitorListener
{
public:
  typedef boost::shared_ptr<IActivityMonitorListener> Ptr;

  virtual ~IActivityMonitorListener() {}

  // Notification that the user is currently active.
  virtual bool action_notify() = 0;
};

class ActivityMonitor
{
public:
  typedef boost::shared_ptr<ActivityMonitor> Ptr;

public:
  ActivityMonitor();
  virtual ~ActivityMonitor();

  virtual void init();
  virtual void terminate();
  virtual void suspend();
  virtual void resume();
  virtual void force_idle();
  virtual bool is_active();
  virtual void set_listener(IActivityMonitorListener::Ptr l);
};

#endif // LOCALACTIVITYMONITOR_HH
