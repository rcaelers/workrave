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

#include <stdio.h>

#include "BreakWindow.hh"
#include "BreakWindowInterface.hh"

class DailyLimitWindow :
  public BreakWindow,
  public BreakWindowInterface
{
public:
  DailyLimitWindow(bool ignorable, bool insist MULTIHEAD_PARAMS);
  virtual ~DailyLimitWindow();

  void start();
  void stop();
  void destroy();
  void set_progress(int value, int max_value);
  void refresh();
  void set_response(BreakResponseInterface *bri);
  
protected:
  void on_postpone_button_clicked();
  void on_skip_button_clicked();
  
private:
  bool insist_break;

  //! Send response to this interface.
  BreakResponseInterface *break_response;
};

inline void
DailyLimitWindow::set_response(BreakResponseInterface *bri)
{
  break_response = bri;
}

#endif // DAILYLIMITWINDOW_HH
