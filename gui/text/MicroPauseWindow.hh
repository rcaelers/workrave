// MicroPauseWindow.hh --- window for the micropause
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef MICROPAUSEWINDOW_HH
#define MICROPAUSEWINDOW_HH

#include "BreakInterface.hh"
#include "BreakWindowInterface.hh"
#include "TimerInterface.hh"

class MicroPauseWindow :
  public BreakWindowInterface
{
public:
  MicroPauseWindow(TimerInterface *timer, bool ignorable);
  ~MicroPauseWindow();

  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void set_insist_break(bool insist);
  void heartbeat();
  void refresh();
private:
  //!
  bool insist_break;
};

#endif // MICROPAUSEWINDOW_HH
