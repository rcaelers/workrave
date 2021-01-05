// TimerBoxtTextView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXTEXTVIEW_HH
#define TIMERBOXTEXTVIEW_HH

#include "preinclude.h"

#include <string>

#include "ITimerBoxView.hh"

class TimerBoxTextView : public ITimerBoxView
{
public:
  TimerBoxTextView();
  virtual ~TimerBoxTextView();

  void set_slot(BreakId id, int slot);
  void set_time_bar(BreakId id,
                    std::string text,
                    ITimeBar::ColorId primary_color,
                    int primary_value,
                    int primary_max,
                    ITimeBar::ColorId secondary_color,
                    int secondary_value,
                    int secondary_max);
  void set_tip(std::string tip);
  void set_icon(IconType icon);
  void update_view();
  void set_enabled(bool enabled);
  void set_geometry(Orientation orientation, int size);

private:
};

#endif // TIMERBOXTEXTVIEW_HH
