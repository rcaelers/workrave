// Copyright (C) 2001, 2002, 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef DAYTIMEPRED_HH
#define DAYTIMEPRED_HH

#include "TimePred.hh"

class DayTimePred : public TimePred
{
public:
  bool init(std::string spec);
  time_t get_next(time_t last_time) override;

private:
  bool init(int hour, int min);
  int days_in_month(int month, int year);
  int time_cmp(int h1, int m1, int h2, int m2);

  int pred_hour{0};
  int pred_min{0};
};

#endif // DAYTIMEPRED_HH
