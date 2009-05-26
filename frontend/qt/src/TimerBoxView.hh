// TimerBoxView.hh --- Timers
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

#ifndef TIMERBOXVIEW_HH
#define TIMERBOXVIEW_HH

#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>

#include "ITimerBoxView.hh"
#include "ICore.hh"
#include "TimeBar.hh"

class TimerBoxView : public QWidget, public ITimerBoxView
{
public:
  TimerBoxView();
  ~TimerBoxView();

  virtual void set_slot(BreakId  id, int slot);
  virtual void set_time_bar(BreakId id,
                            std::string text,
                            ITimeBar::ColorId primary_color,
                            int primary_value, int primary_max,
                            ITimeBar::ColorId secondary_color,
                            int secondary_value, int secondary_max);
  virtual void set_tip(std::string tip);
  virtual void set_icon(IconType icon);
  virtual void update_view();
  virtual void set_enabled(bool enabled);

private:
  QGridLayout *layout;
  QLabel *break_labels[BREAK_ID_SIZEOF];
  TimeBar *break_bars[BREAK_ID_SIZEOF];
};

#endif // ITIMERBOXVIEW_HH
