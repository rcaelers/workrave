// TimerBoxView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
// All rights reserved.
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

#ifndef TIMERBOXVIEW_HH
#define TIMERBOXVIEW_HH

#include <string>

#include "TimeBarInterface.hh"
#include "CoreInterface.hh"


class TimerBoxView
{
public:
  virtual void set_slot(BreakId  id, int slot) = 0;
  virtual void set_time_bar(BreakId id,
                            std::string text
                            , TimeBarInterface::ColorId primary_color,
                            int primary_value, int primary_max,
                            TimeBarInterface::ColorId secondary_color,
                            int secondary_value, int secondary_max) = 0;
  virtual void update() = 0;
  
};

#endif // TIMERBOXVIEW_HH
