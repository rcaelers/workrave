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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <cstdlib>
#include <string>
#include <cstdio>
#include <ctime>

#include "DayTimePred.hh"

using namespace std;

int
DayTimePred::time_cmp(int h1, int m1, int h2, int m2)
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
DayTimePred::init(int hour, int min)
{
  pred_hour = hour;
  pred_min = min;

  return true;
}

bool
DayTimePred::init(std::string spec)
{
  bool ret = false;
  std::string::size_type pos = spec.find(':');

  if (pos != std::string::npos)
    {
      std::string hours;
      std::string minutes;

      hours = spec.substr(0, pos);
      minutes = spec.substr(pos + 1);

      ret = init(atoi(hours.c_str()), atoi(minutes.c_str()));
    }

  return ret;
}

int
DayTimePred::days_in_month(int month, int year)
{
  if (month == 1)
    {
      // Feb

      if (year % 4 == 0 && ((year % 100 != 0) || (year % 400) == 0))
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
      int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      return days[month];
    }
}

int64_t
DayTimePred::get_time_offset()
{
  return pred_hour * 60 * 60 + pred_min * 60;
}

int64_t
DayTimePred::get_next(int64_t last_time)
{
  struct tm *ret;

  // FIXME:
  time_t t = (time_t)last_time;
  ret = localtime(&t);

  if (ret != nullptr)
    {
      if (time_cmp(ret->tm_hour, ret->tm_min, pred_hour, pred_min) >= 0)
        {
          ret->tm_mday++;
        }

      ret->tm_sec = 0;
      ret->tm_min = pred_min;
      ret->tm_hour = pred_hour;

      if (ret->tm_mday > days_in_month(ret->tm_mon, ret->tm_year + 1900))
        {
          ret->tm_mday = 1;
          ret->tm_mon++;
        }

      if (ret->tm_mon > 11)
        {
          ret->tm_mon = 0;
          ret->tm_year++;
        }

      return mktime(ret);
    }

  return 0;
}

string
DayTimePred::to_string() const
{
  char buf[16];
  sprintf(buf, "day/%d:%02d", pred_hour, pred_min);
  return string(buf);
}
