// DailyLimitWindow.hh --- window for the daily limit
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

#include <stdio.h>

#include "BreakInterface.hh"
#include "BreakWindow.hh"
#include "BreakWindowInterface.hh"

class TimerInterface;
class ControlInterface;
class Frame;

class DailyLimitWindow :
  public BreakWindow,
  public BreakWindowInterface
{
public:
  DailyLimitWindow(TimerInterface *timer, bool ignorable);
  ~DailyLimitWindow();

  void start();
  void stop();
  void destroy();
  void set_insist_break(bool insist);
  void heartbeat();
  
protected:
  void on_postpone_button_clicked();
  void on_skip_button_clicked();
  
private:
  void refresh();
  void refresh_time_bar();
  void refresh_label();
  
private:
  //!
  TimerInterface *restbreak_timer;
  
  //!
  bool insist_break;
};

#endif // DAILYLIMITWINDOW_HH
