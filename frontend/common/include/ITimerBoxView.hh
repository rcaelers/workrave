// ITimerBoxView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

#ifndef ITIMERBOXVIEW_HH
#define ITIMERBOXVIEW_HH

#include <string>

#include "ITimeBar.hh"
#include "ICore.hh"
#include "Orientation.hh"

using namespace workrave;

class ITimerBoxView
{
public:
  enum IconType
  {
    ICON_NORMAL,
    ICON_QUIET,
    ICON_SUSPENDED
  };

  virtual ~ITimerBoxView() {}

  virtual void set_slot(BreakId id, int slot) = 0;
  virtual void set_time_bar(BreakId id,
                            std::string text,
                            ITimeBar::ColorId primary_color,
                            int primary_value,
                            int primary_max,
                            ITimeBar::ColorId secondary_color,
                            int secondary_value,
                            int secondary_max) = 0;
  virtual void set_tip(std::string tip) = 0;
  virtual void set_icon(IconType icon) = 0;
  virtual void update_view() = 0;
  virtual void set_geometry(Orientation orientation, int size) = 0;
};

#endif // ITIMERBOXVIEW_HH
