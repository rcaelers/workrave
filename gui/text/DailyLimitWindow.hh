// DailyLimitWindow.hh --- window for the daily limit
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

#ifndef DAILYLIMITWINDOW_HH
#define DAILYLIMITWINDOW_HH

#include "BreakInterface.hh"
#include "BreakWindowInterface.hh"

class DailyLimitWindow :
  public BreakWindowInterface
{
public:
  DailyLimitWindow(bool ignorable);
  ~DailyLimitWindow();

  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void set_insist_break(bool insist);
  void refresh();
  
private:
  bool insist_break;
};

#endif // DAILYLIMITWINDOW_HH
