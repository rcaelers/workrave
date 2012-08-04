// TimeSource.hh --- The Time
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef TIMESOURCE_HH
#define TIMESOURCE_HH

#include "ITimeSource.hh"

namespace workrave
{
  namespace utils
  {
    //! A source of time.
    class TimeSource
    {
    public:
      static const gint64 USEC_PER_SEC = G_USEC_PER_SEC;
      
      //! Returns the system wall-clock time.
      static gint64 get_real_time_usec();

      //! Returns the system monotonic time, if available.
      static gint64 get_monotonic_time_usec();

      //! Returns the system wall-clock time.
      static gint64 get_real_time();

      //! Returns the system monotonic time, if available.
      static gint64 get_monotonic_time();
      
    public:
      static ITimeSource::Ptr source;
    };
  }
}

#endif // TIMESOURCE_HH
