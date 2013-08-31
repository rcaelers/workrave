// ActivityMonitor.hh --- ActivityMonitor functionality
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2010, 2013 Rob Caelers <robc@krandor.nl>
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

#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"

#include "LocalActivityMonitor.hh"
#include "CoreHooks.hh"

using namespace workrave::config;

class ActivityMonitor :
  public IActivityMonitor
{
public:
  typedef boost::shared_ptr<ActivityMonitor> Ptr;

public:
  static Ptr create(IConfigurator::Ptr configurator, CoreHooks::Ptr hooks, const std::string &display_name);

  ActivityMonitor(IConfigurator::Ptr configurator, CoreHooks::Ptr hooks, const std::string &display_name);

  virtual void init();
  virtual void terminate();
  virtual void suspend();
  virtual void resume();
  virtual void force_idle();
  virtual ActivityState get_state();
  virtual void set_listener(IActivityMonitorListener::Ptr l);
  
  void heartbeat();
  void report_external_activity(std::string who, bool act);

private:
  //! The Configurator.
  IConfigurator::Ptr configurator;

  //! Hooks
  CoreHooks::Ptr hooks;

  //! Activity listener.
  IActivityMonitorListener::Ptr listener;

  //! The activity monitor
  IActivityMonitor::Ptr local_monitor;

  //! External activity
  std::map<std::string, int64_t> external_activity;

  //! Current overall monitor state.
  ActivityState local_state;

  //! Current overall monitor state.
  ActivityState monitor_state;
};

#endif // ACTIVITYMONITOR_HH
