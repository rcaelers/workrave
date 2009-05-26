// HeadInfo.hh --- Multi head info
//
// Copyright (C) 2001, 2002, 2003, 2004, 2007 Rob Caelers & Raymond Penners
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

#ifndef MULTIHEAD_HH
#define MULTIHEAD_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#ifdef PLATFORM_OS_WIN32_NATIVE
#undef max
#endif

#include <gdkmm/rectangle.h>
#include <gdkmm/display.h>
#include <gdkmm/screen.h>

class HeadInfo
{
public:
  HeadInfo()
  {
    valid = false;
    count = 0;
  }

  int get_width() const;
  int get_height() const;
  int get_x() const;
  int get_y() const;

  Glib::RefPtr<Gdk::Screen> screen;
  int monitor;
  int count;
  bool valid;
  Gdk::Rectangle geometry;
};

#endif // MULTIHEAD_HH
