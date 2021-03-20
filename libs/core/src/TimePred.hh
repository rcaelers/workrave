// TimePred.hh --- Time Predicate
//
// Copyright (C) 2001, 2002, 2003, 2005, 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef TIMEPRED_HH
#define TIMEPRED_HH

#if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
#  if HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

//! A time predicate.
/*! Given a previous time that matched, it computes the next time that matches
 *  a certain time-predicate. This is used for, for example, daily limits.
 *  Given the last daily limit (or the current time if there was no previous
 *  daily limit), a time predicate computes the time of the next daily limit.
 *
 *  This is base class for all time predicates (e.g. daily, weekly,...)
 */
class TimePred
{
public:
  virtual ~TimePred() = default;

  //! Set the last time the predicate matched.
  virtual void set_last(time_t lastTime) { last_time = lastTime; }

  //! Computes the next time the predicate matches given the time of the previous match.
  virtual time_t get_next() = 0;

  //! Returns the string representation of this predicate.
  virtual std::string to_string() const = 0;

protected:
  //! Last time the predicate matched.
  time_t last_time{0};
};

#endif // TIMEPRED_HH
