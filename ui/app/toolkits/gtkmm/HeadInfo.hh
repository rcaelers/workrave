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

#ifndef HEADINFO_HH
#define HEADINFO_HH

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
#  undef max
#endif

#include <gdkmm/rectangle.h>
#include "gdkmm/monitor.h"

class HeadInfo
{
public:
  HeadInfo() = default;

  int get_width() const;
  int get_height() const;
  int get_x() const;
  int get_y() const;
  bool is_primary() const;
  Glib::RefPtr<Gdk::Monitor> get_monitor() const;

  bool primary{false};
  Gdk::Rectangle geometry;
  Glib::RefPtr<Gdk::Monitor> monitor;
};

#endif // HEADINFO_HH
