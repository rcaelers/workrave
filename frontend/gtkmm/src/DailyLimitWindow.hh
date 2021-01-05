// DailyLimitWindow.hh --- window for the daily limit
//
// Copyright (C) 2001, 2002, 2003, 2007, 2008 Rob Caelers & Raymond Penners
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

#ifndef DAILYLIMITWINDOW_HH
#define DAILYLIMITWINDOW_HH

#include <stdio.h>

#include "BreakWindow.hh"
#include "GUIConfig.hh"

class DailyLimitWindow : public BreakWindow
{
public:
  DailyLimitWindow(HeadInfo &head, BreakFlags break_flags, GUIConfig::BlockMode mode);
  virtual ~DailyLimitWindow();

  void set_progress(int value, int max_value);

protected:
  Gtk::Widget *create_gui();
};

#endif // DAILYLIMITWINDOW_HH
