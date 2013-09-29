// Copyright (C) 2013 Rob Caelers
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

#include <boost/enable_shared_from_this.hpp>

//#include <boost/date_time/local_time/local_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"

class SimulatedTime : public workrave::utils::ITimeSource, public boost::enable_shared_from_this<SimulatedTime>
{
public:
  typedef boost::shared_ptr<SimulatedTime> Ptr;

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
    boost::posix_time::ptime then(boost::gregorian::date(2002,9,22), boost::posix_time::time_duration(22,00,00));
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1)); 
    boost::posix_time::time_duration diff = then - epoch;
    current_time = diff.total_microseconds();
  }
  
  virtual int64_t get_real_time_usec()
  {
    return current_time;
  }
  
  virtual int64_t get_monotonic_time_usec()
  {
    return current_time;
  }

  int64_t current_time;
  
private:
  void init()
  {
    reset();
    workrave::utils::TimeSource::source = shared_from_this();
  }
  
  static SimulatedTime::Ptr instance;
};

#endif
