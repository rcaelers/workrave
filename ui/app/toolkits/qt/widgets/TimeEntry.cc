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
#  include "config.h"
#endif

#include "debug.hh"

#include "TimeEntry.hh"

TimeEntry::TimeEntry()
{
  secs = new QSpinBox;
  secs->setWrapping(true);
  secs->setMinimum(0);
  secs->setMaximum(59);

  hrs = new QSpinBox;
  hrs->setWrapping(true);
  hrs->setMinimum(0);
  hrs->setMaximum(59);

  mins = new QSpinBox;
  mins->setWrapping(true);
  mins->setMinimum(0);
  mins->setMaximum(59);

  void (QSpinBox::*signal)(int) = &QSpinBox::valueChanged;
  connect(secs, signal, this, &TimeEntry::on_value_changed);
  connect(hrs, signal, this, &TimeEntry::on_value_changed);
  connect(mins, signal, this, &TimeEntry::on_value_changed);

  auto *semi1 = new QLabel(":");
  auto *semi2 = new QLabel(":");

  auto *layout = new QHBoxLayout;
  setLayout(layout);

  layout->addWidget(hrs, 1);
  layout->addWidget(semi1);
  layout->addWidget(mins, 1);
  layout->addWidget(semi2);
  layout->addWidget(secs, 1);

  layout->setContentsMargins(1, 1, 1, 1);
  layout->setSpacing(2);
}

void
TimeEntry::set_value(time_t t)
{
  hrs->setValue(static_cast<double>(t / (60 * 60)));
  mins->setValue(static_cast<double>((t / 60) % 60));
  secs->setValue(static_cast<double>(t % 60));
}

auto
TimeEntry::get_value() -> time_t
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

auto
TimeEntry::signal_value_changed() -> boost::signals2::signal<void()> &
{
  return value_changed_signal;
}
