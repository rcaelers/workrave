// ActivityMonitorStub.hh --- ActivityMonitorStub functionality
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

#ifndef ACTIVITYMONITORSTUB_HH
#define ACTIVITYMONITORSTUB_HH

#include "IActivityMonitor.hh"

class ActivityMonitorStub : public IActivityMonitor
{
public:
  typedef boost::shared_ptr<ActivityMonitorStub> Ptr;

public:
  static Ptr create();

  ActivityMonitorStub();
  virtual ~ActivityMonitorStub();

  void set_active(bool active);

  virtual void init();
  virtual void terminate();
  virtual void suspend();
  virtual void resume();
  virtual void force_idle();
  virtual bool is_active();
  virtual void set_listener(IActivityMonitorListener::Ptr l);

  void notify();

private:
  bool active;
  bool suspended;
  bool forced_idle;
  IActivityMonitorListener::Ptr listener;
};

#endif // LOCALACTIVITYMONITOR_HH
