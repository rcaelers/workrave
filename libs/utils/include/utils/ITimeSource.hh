// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_ITIMESOURCE_HH
#define WORKRAVE_UTILS_ITIMESOURCE_HH

#include <cstdint>
#include <memory>

namespace workrave::utils
{
  //! A source of time.
  class ITimeSource
  {
  public:
    using Ptr = std::shared_ptr<ITimeSource>;

    virtual ~ITimeSource() = default;

    //! Returns the system wall-clock time.
    virtual int64_t get_real_time_usec() = 0;

    //! Returns the system monotonic time, if available.
    virtual int64_t get_monotonic_time_usec() = 0;
  };
} // namespace workrave::utils

#endif // WORKRAVE_UTILS_ITIMESOURCE_HH
