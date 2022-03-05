// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#include "Time.hh"

using namespace ui::prefwidgets;

Time::Time(const std::string &label, int min, int max, TimeKind kind)
  : WidgetBase<Time, int>(label)
  , min(min)
  , max(max)
  , kind(kind)
{
}

std::shared_ptr<Time>
Time::create(const std::string &label, int min, int max, TimeKind kind)
{
  return std::make_shared<Time>(label, min, max, kind);
}

int
Time::get_min() const
{
  return min;
}

int
Time::get_max() const
{
  return max;
}

TimeKind
Time::get_kind() const
{
  return kind;
}
