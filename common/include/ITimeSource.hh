// ITimeSource.hh --- The Time
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef ITIMESOURCE_HH
#define ITIMESOURCE_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

//! A source of time.
class ITimeSource
{
public:
  virtual ~ITimeSource() {}

  //! Returns the time of this source.
  virtual time_t get_time() const = 0;
};

#endif // ITIMESOURCE_HH
