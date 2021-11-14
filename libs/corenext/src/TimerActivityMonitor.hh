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

#ifndef TIMERACTIVITYMONITOR_HH
#define TIMERACTIVITYMONITOR_HH

#include <memory>

#include "IActivityMonitor.hh"
#include "Timer.hh"

class TimerActivityMonitor
{
public:
  using Ptr = std::shared_ptr<TimerActivityMonitor>;

public:
  TimerActivityMonitor(IActivityMonitor::Ptr monitor, Timer::Ptr timer);
  virtual ~TimerActivityMonitor() = default;

  void suspend();
  void resume();
  bool is_active();
  void force_idle();

private:
  IActivityMonitor::Ptr monitor;
  Timer::Ptr timer;
  bool suspended;
  bool forced_idle;
};

#endif // TIMERACTIVITYMONITOR_HH
