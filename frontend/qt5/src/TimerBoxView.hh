// TimerBoxView.hh
//
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

#include <QLabel>
#include <QGridLayout>

#include "ITimerBoxView.hh"
#include "TimeBar.hh"

class TimerBoxView : public QWidget, public ITimerBoxView
{
  Q_OBJECT

public:
  TimerBoxView();
  virtual ~TimerBoxView();

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
  virtual void set_geometry(Orientation orientation, int size);

private:
  void init_widgets();
  void init_table();
  void init();

  int get_number_of_timers() const;
  bool is_sheep_only() const;
  void set_sheep_only(bool sheep_only);
  
  //!
  QGridLayout *layout;
  
  //! Array of time labels
  QLabel *labels[BREAK_ID_SIZEOF];

  //! Array of time bar widgets.
  TimeBar *bars[BREAK_ID_SIZEOF];

  //! Current slot content.
  int content[BREAK_ID_SIZEOF];

  //! Only show the sheep
  bool sheep_only;

  //!
  bool reconfigure;
};

#endif // TIMERBOXVIEW_HH
