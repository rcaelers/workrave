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

#ifndef SIMULATEDTIME_HH
#define SIMULATEDTIME_HH

#include <chrono>

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"

class SimulatedTime
  : public workrave::utils::ITimeSource
  , public std::enable_shared_from_this<SimulatedTime>
{
public:
  typedef std::shared_ptr<SimulatedTime> Ptr;

  static Ptr create()
  {
    if (!instance)
      {
        instance = SimulatedTime::Ptr(new SimulatedTime());
        instance->init();
      }
    return instance;
  }

  void reset()
  {
    std::tm tm;
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 22;
    tm.tm_mday = 22;
    tm.tm_mon = 9;
    tm.tm_year = 102;
    tm.tm_isdst = -1;
    std::time_t tt = std::mktime(&tm);

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(tt);
    current_time = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
  }

  int64_t get_real_time_usec() override
  {
    return current_time;
  }

  int64_t get_monotonic_time_usec() override
  {
    return current_time;
  }

  int64_t current_time;

private:
  SimulatedTime() = default;

  void init()
  {
    reset();
    workrave::utils::TimeSource::source = shared_from_this();
  }

  static SimulatedTime::Ptr instance;
};

#endif
