// WeekTimePred.cc --- Daily Time Predicate
//
// Copyright (C) 2001, 2002, 2003, 2007 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>
#include <string>
#include <stdio.h>

#include "WeekTimePred.hh"

using namespace std;

//! Set the last time the predicate matched.
void
WeekTimePred::set_last(time_t lastTime)
{
  last_time = lastTime;

  // FIXME: use core to ask time.
  time_t now = time(NULL);

  if (last_time == 0)
    {
      last_time = now;
    }
}


int
WeekTimePred::time_cmp(int h1, int m1, int h2, int m2)
{
  if (h1 < h2)
    return -1;
  else if (h1 > h2)
    return 1;

  if (m1 < m2)
    return -1;
  else if (m1 > m2)
    return 1;

  return 0;
}


bool
WeekTimePred::init(int day, int hour, int min)
{
  pred_day = day;
  pred_hour = hour;
  pred_min = min;

  return true;
}


bool
WeekTimePred::init(std::string spec)
{
  bool ret = false;
  std:: string::size_type pos1 = spec.find(':');

  if (pos1 != std::string::npos)
    {
      std:: string::size_type pos2 = spec.find(pos1 + 1, ':');

      if (pos2 != std::string::npos)
        {
          std::string day;
          std::string hours;
          std::string minutes;

          day = spec.substr(0, pos1);
          minutes = spec.substr(pos1 + 1, pos2);
          hours = spec.substr(pos2 + 1);

          ret = init(atoi(day.c_str()), atoi(hours.c_str()), atoi(minutes.c_str()));

        }
    }

  return ret;
}


int
WeekTimePred::days_in_month(int month, int year)
{
  int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (month == 1)
    {
      // Feb

      if (year % 4 == 0 && ( (year % 100 != 0) || (year % 400) == 0))
        {
          return 29;
        }
      else
        {
          return 28;
        }
    }
  else
    {
      return days[month];
    }
}


time_t
WeekTimePred::get_next()
{
  struct tm *ret;

  ret = localtime(&last_time);

  if (ret != NULL)
    {
      int wdayDiff = pred_day - ret->tm_wday;
      if ( ( wdayDiff < 0 )  ||
           ( (wdayDiff == 0) && (time_cmp(ret->tm_hour, ret->tm_min, pred_hour, pred_min) >= 0) ) )
        {
          wdayDiff += 7;
        }

      ret->tm_mday += wdayDiff;
      ret->tm_wday = pred_day;

      ret->tm_sec = 0;
      ret->tm_min = pred_min;
      ret->tm_hour = pred_hour;

      int monthDays = days_in_month(ret->tm_mon, ret->tm_year + 1900);
      if (ret->tm_mday > monthDays)
        {
          ret->tm_mday -= monthDays;
          ret->tm_mon++;
        }

      if (ret->tm_mon > 11)
        {
          ret->tm_mon = 0;
          ret->tm_year++;
        }

      return mktime(ret);
    }
  else
    {
      return 0;
    }
}


string
WeekTimePred::to_string() const
{
  // FIXME
  abort();
}
