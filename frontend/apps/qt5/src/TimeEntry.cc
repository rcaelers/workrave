// TimeEntry.cc --- Entry widget for time
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "TimeEntry.hh"

TimeEntry::TimeEntry()
{
  secs = new QSpinBox;
  secs->setWrapping(true);
  //secs->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  secs->setMinimum(0);
  secs->setMaximum(59);

  hrs = new QSpinBox;
  hrs->setWrapping(true);
  //hrs->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  hrs->setMinimum(0);
  hrs->setMaximum(59);

  mins = new QSpinBox;
  mins->setWrapping(true);
  //mins->setButtonSymbols(QAbstractSpinBox::PlusMinus);
  mins->setMinimum(0);
  mins->setMaximum(59);

  void (QSpinBox:: *signal)(int) = &QSpinBox::valueChanged;
  connect(secs, signal, this, &TimeEntry::on_value_changed);
  connect(hrs, signal, this, &TimeEntry::on_value_changed);
  connect(mins, signal, this, &TimeEntry::on_value_changed);

  QLabel *semi1 = new QLabel(":");
  QLabel *semi2 = new QLabel(":");

  QHBoxLayout *layout = new QHBoxLayout;
  setLayout(layout);

  layout->addWidget(hrs);
  layout->addWidget(semi1);
  layout->addWidget(mins);
  layout->addWidget(semi2);
  layout->addWidget(secs);
}


//! Destructor.
TimeEntry::~TimeEntry()
{
}


//! Set time
void
TimeEntry::set_value(time_t t)
{
  hrs->setValue((double)(t / (60*60)));
  mins->setValue((double)((t / 60) % 60));
  secs->setValue((double)(t % 60));
}

//! Get time
time_t
TimeEntry::get_value()
{
  int s = secs->value();
  int h = hrs->value();
  int m = mins->value();
  return h * 60 * 60 + m * 60 + s;
}

void
TimeEntry::on_value_changed()
{
  value_changed_signal();
}


boost::signals2::signal<void()> &
TimeEntry::signal_value_changed()
{
  return value_changed_signal;
}

