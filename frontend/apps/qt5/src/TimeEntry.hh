// TimeEntry.hh --- Entry widget for time
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
  virtual ~TimeEntry();

  time_t get_value();
  void set_value(time_t time);

  boost::signals2::signal<void()> &signal_value_changed();

private:
  void on_value_changed();

private:
  //! Value changed
  boost::signals2::signal<void()> value_changed_signal;

   QSpinBox *hrs;
   QSpinBox *mins;
   QSpinBox *secs;
};

#endif // TIMEENTRY_HH
