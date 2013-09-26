// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef READINGACTIVITYMONITOR_HH
#define READINGACTIVITYMONITOR_HH

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Break.hh"
#include "CoreModes.hh"
#include "ActivityMonitor.hh"
#include "Timer.hh"

using namespace workrave;

class ReadingActivityMonitor :
  public IActivityMonitorListener,
  public boost::enable_shared_from_this<ReadingActivityMonitor>
{
public:
  typedef boost::shared_ptr<ReadingActivityMonitor> Ptr;

public:
  static Ptr create(ActivityMonitor::Ptr monitor, CoreModes::Ptr modes);

  ReadingActivityMonitor(ActivityMonitor::Ptr monitor, CoreModes::Ptr modes);
  virtual ~ReadingActivityMonitor();

  void handle_break_event(BreakId break_id, BreakEvent event);

  void init();
  void suspend();
  void resume();
  void force_idle();
  bool is_active();

private:
  bool action_notify();
  void on_usage_mode_changed(workrave::UsageMode mode); 
  
private:
  enum State { Idle, Active, Prelude, Taking };
    
  ActivityMonitor::Ptr monitor;
  CoreModes::Ptr modes;
  bool suspended;
  bool forced_idle;
  State state;
};

#endif // READINGACTIVITYMONITOR_HH

