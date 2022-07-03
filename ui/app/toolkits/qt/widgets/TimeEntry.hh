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

#ifndef TIMEENTRY_HH
#define TIMEENTRY_HH

#include <boost/signals2.hpp>

#include <QtGui>
#include <QtWidgets>

class TimeEntry : public QWidget
{
  Q_OBJECT

public:
  TimeEntry();

  auto get_value() -> time_t;
  void set_value(time_t time);

  auto signal_value_changed() -> boost::signals2::signal<void()> &;

private:
  void on_value_changed();

private:
  boost::signals2::signal<void()> value_changed_signal;

  QSpinBox *hrs{nullptr};
  QSpinBox *mins{nullptr};
  QSpinBox *secs{nullptr};
};

#endif // TIMEENTRY_HH
