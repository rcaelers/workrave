// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_UI_ITIMERBOXVIEW_HH
#define WORKRAVE_UI_ITIMERBOXVIEW_HH

#include "core/CoreTypes.hh"
#include "ui/UiTypes.hh"

class ITimerBoxView
{
public:
  virtual ~ITimerBoxView() = default;

  virtual void set_slot(workrave::BreakId id, int slot) = 0;
  virtual void set_time_bar(workrave::BreakId id,
                            int value,
                            TimerColorId primary_color,
                            int primary_value,
                            int primary_max,
                            TimerColorId secondary_color,
                            int secondary_value,
                            int secondary_max) = 0;
  virtual void set_icon(OperationModeIcon icon) = 0;
  virtual void set_geometry(Orientation orientation, int size) = 0;
  virtual void update_view() = 0;
};

#endif // WORKRAVE_UI_ITIMERBOXVIEW_HH
