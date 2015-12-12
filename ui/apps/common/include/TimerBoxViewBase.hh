// TimerBoxViewBase.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXVIEWBASE_HH
#define TIMERBOXVIEWBASE_HH

#include <string>

#include "ITimerBoxView.hh"

class TimerBoxViewBase : public ITimerBoxView
{
public:
  ~TimerBoxViewBase() override {}

  void set_tip(std::string tip) override
  {
    (void) tip;
  }

  void set_icon(StatusIconType icon) override
  {
    (void) icon;
  }

  void update_view() override
  {
  }

  void set_enabled(bool enabled) override
  {
    (void) enabled;
  }

  void set_geometry(Orientation orientation, int size) override
  {
    (void) orientation;
    (void) size;
  }
};

#endif // TIMERBOXVIEWBASE_HH
