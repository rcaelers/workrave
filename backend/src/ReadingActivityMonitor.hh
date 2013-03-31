// ReadingActivityMonitor.hh
//
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

#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"
#include "Timer.hh"

using namespace workrave;

//! An Activity Monitor that takes its activity state from a timer.
/*! This Activity Monitor is 'active' if the timer is running or if
 *  the timer has its idle time not at maximum, 'idle' otherwise.
 */
class ReadingActivityMonitor :
  public IActivityMonitor,
  public IActivityMonitorListener,
  public boost::enable_shared_from_this<ReadingActivityMonitor>
{
public:
  typedef boost::shared_ptr<ReadingActivityMonitor> Ptr;

public:
  static Ptr create(IActivityMonitor::Ptr monitor);

  ReadingActivityMonitor(IActivityMonitor::Ptr monitor);
  virtual ~ReadingActivityMonitor();

  void handle_break_event(BreakId break_id, IBreak::BreakEvent event);
  
  // IActivityMonitor
  virtual void init();
  virtual void terminate();
  virtual void suspend();
  virtual void resume();
  virtual ActivityState get_state();
  virtual void force_idle();
  virtual void set_listener(IActivityMonitorListener::Ptr l);

private:
  bool action_notify();
  
private:
  enum State { Idle, Active, Prelude, Taking };
    
  //! Reference monitor
  IActivityMonitor::Ptr monitor;

  //! Monitor suspended?
  bool suspended;

  //! Is this timer forced idle?
  bool forced_idle;

  //!
  State state;
};

#endif // READINGACTIVITYMONITOR_HH
