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
#include "GUI.hh"

class DailyLimitWindow :
  public BreakWindow
{
public:
  DailyLimitWindow(HeadInfo &head, bool ignorable, GUI::BlockMode mode);
  virtual ~DailyLimitWindow();

  void set_progress(int value, int max_value);

protected:
  Gtk::Widget *create_gui();
};


#endif // DAILYLIMITWINDOW_HH
