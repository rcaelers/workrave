// DayTimePred.hh --- Daily Time Predicate
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-10-15 09:36:38 robc>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef DAYTIMEPRED_HH
#define DAYTIMEPRED_HH

#include "TimePred.hh"

class DayTimePred : public TimePred
{
public:
  bool init(string spec);
  bool init(int hour, int min);

  void set_last(time_t lastTime);
  time_t get_next();
  time_t get_time_offset();
  string to_string() const;

private:
  int days_in_month(int month, int year);
  int time_cmp(int h1, int m1, int h2, int m2);

  int pred_hour;
  int pred_min;
};


#endif // DAYTIMEPRED_HH
