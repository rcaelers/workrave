// WeekTimePred.hh --- Weekly Time Predicate
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.nl>
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

#ifndef WEEKTIMEPRED_HH
#define WEEKTIMEPRED_HH

#include "TimePred.hh"

class WeekTimePred : public TimePred
{
public:
  bool init(std::string spec);
  bool init(int day, int hour, int min);

  void set_last(time_t lastTime);
  time_t get_next();
  std::string to_string() const;

private:
  int days_in_month(int month, int year);
  int time_cmp(int h1, int m1, int h2, int m2);

  int pred_day;
  int pred_hour;
  int pred_min;
};


#endif // WEEKTIMEPRED_HH
