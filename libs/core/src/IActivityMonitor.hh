// Copyright (C) 2001 - 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef IACTIVITYMONITOR_HH
#define IACTIVITYMONITOR_HH

#include <string>
#include <iostream>
#include <memory>

class IActivityMonitorListener;

//! State of the activity monitor.
enum ActivityState
{
  ACTIVITY_UNKNOWN,
  ACTIVITY_SUSPENDED,
  ACTIVITY_IDLE,
  ACTIVITY_NOISE,
  ACTIVITY_ACTIVE
};

inline std::ostream &
operator<<(std::ostream &stream, ActivityState mode)
{
  switch (mode)
    {
    case ACTIVITY_UNKNOWN:
      stream << "unknown";
      break;
    case ACTIVITY_SUSPENDED:
      stream << "suspended";
      break;
    case ACTIVITY_IDLE:
      stream << "idle";
      break;
    case ACTIVITY_NOISE:
      stream << "noise";
      break;
    case ACTIVITY_ACTIVE:
      stream << "active";
      break;
    }
  return stream;
}

class IActivityMonitor
{
public:
  using Ptr = std::shared_ptr<IActivityMonitor>;

public:
  virtual ~IActivityMonitor() = default;

  virtual void terminate() = 0;
  virtual void suspend() = 0;
  virtual void resume() = 0;
  virtual ActivityState get_current_state() = 0;
  virtual void force_idle() = 0;
  virtual void set_listener(IActivityMonitorListener *l) = 0;
};

#endif // IACTIVITYMONITOR_HH
