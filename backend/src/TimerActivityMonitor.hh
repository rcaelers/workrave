// TimerActivityMonitor.hh
//
// Copyright (C) 2001 - 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef TIMERACTIVITYMONITOR_HH
#define TIMERACTIVITYMONITOR_HH

#include <boost/shared_ptr.hpp>

#include "IActivityMonitor.hh"
#include "IActivityMonitorListener.hh"
#include "Timer.hh"

//! An Activity Monitor that takes its activity state from a timer.
/*! This Activity Monitor is 'active' if the timer is running or if
 *  the timer has its idle time not at maximum, 'idle' otherwise.
 */
class TimerActivityMonitor : public IActivityMonitor
{
public:
  typedef boost::shared_ptr<TimerActivityMonitor> Ptr;

public:
  static Ptr create(IActivityMonitor::Ptr monitor, Timer::Ptr timer);

  TimerActivityMonitor(IActivityMonitor::Ptr monitor, Timer::Ptr timer);
  virtual ~TimerActivityMonitor();

  void terminate();
  void suspend();
  void resume();
  ActivityState get_current_state();
  void force_idle();
  void set_listener(IActivityMonitorListener::Ptr l);

private:
  //! Reference monitor
  IActivityMonitor::Ptr monitor;

  //! Reference timer.
  Timer::Ptr timer;

  //! Monitor suspended?
  bool suspended;

  //! Is this timer forced idle?
  bool forced_idle;
};

#endif // TIMERACTIVITYMONITOR_HH
