// TimePredFactory.cc
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-08-11 19:16:30 robc>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TimePredFactory.hh"
#include "DayTimePred.hh"
#include "WeekTimePred.hh"

TimePred *
TimePredFactory::create_time_pred(string spec)
{
  TimePred *pred = 0;
  bool ok = false;
  
  std::string type;
  std::string::size_type pos = spec.find('/');

  if (pos != std::string::npos)
    {
      type = spec.substr(0, pos);
      spec = spec.substr(pos + 1);

      if (type == "day")
        {
          DayTimePred *dayPred = new DayTimePred();
          ok = dayPred->init(spec);
          pred = dayPred;
        }
      else if (type == "week")
        {
          WeekTimePred *weekPred = new WeekTimePred();
          ok = weekPred->init(spec);
          pred = weekPred;
        }
    }

  if (pred && !ok)
    {
      delete pred;
      pred = NULL;
    }
  
  return pred;
}

  
