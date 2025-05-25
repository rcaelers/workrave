// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include <string>

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "ui/ITimerBoxView.hh"

#include "TimeBar.hh"

class TimerBoxView
  : public QWidget
  , public ITimerBoxView
{
  Q_OBJECT

public:
  TimerBoxView();
  ~TimerBoxView() override;

  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override;
  void update_view() override;

private:
  void init_table();
  void init();

  auto get_number_of_timers() const -> int;
  auto is_sheep_only() const -> bool;
  void set_sheep_only(bool sheep_only);

private:
  QGridLayout *layout{nullptr};
  QWidget *labels[workrave::BREAK_ID_SIZEOF];
  TimeBar *bars[workrave::BREAK_ID_SIZEOF];
  QLabel *sheep{nullptr};
  bool reconfigure{true};
  int size{0};
  int current_content[workrave::BREAK_ID_SIZEOF];
  int new_content[workrave::BREAK_ID_SIZEOF];
  int visible_count{-1};
  bool sheep_only{false};
};

#endif // TIMERBOXVIEW_HH
