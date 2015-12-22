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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "commonui/TimerBoxViewBase.hh"

#include "TimeBar.hh"

class TimerBoxView : public QWidget, public TimerBoxViewBase
{
  Q_OBJECT

public:
  TimerBoxView();
  ~TimerBoxView() override;

  void set_slot(workrave::BreakId  id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                            std::string text,
                            ITimeBar::ColorId primary_color,
                            int primary_value, int primary_max,
                            ITimeBar::ColorId secondary_color,
                            int secondary_value, int secondary_max) override;

  void set_icon(StatusIconType icon) override;
  void update_view() override;
  void set_enabled(bool enabled) override;

private:
  void init_table();
  void init();

  int get_number_of_timers() const;
  bool is_sheep_only() const;
  void set_sheep_only(bool sheep_only);

private:  
  QGridLayout *layout;
  QWidget *labels[workrave::BREAK_ID_SIZEOF];
  TimeBar *bars[workrave::BREAK_ID_SIZEOF];
  QLabel *sheep;
  bool reconfigure;
  int size;
  int current_content[workrave::BREAK_ID_SIZEOF];
  int new_content[workrave::BREAK_ID_SIZEOF];
  int visible_count;
  bool sheep_only;
};

#endif // TIMERBOXVIEW_HH
