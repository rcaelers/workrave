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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "HeadInfo.hh"

#include <gdkmm/display.h>

int
HeadInfo::get_width() const
{
  return geometry.get_width();
}

int
HeadInfo::get_height() const
{
  return geometry.get_height();
}

int
HeadInfo::get_x() const
{
  return geometry.get_x();
}

int
HeadInfo::get_y() const
{
  return geometry.get_y();
}

bool
HeadInfo::is_primary() const
{
  return primary;
}

Glib::RefPtr<Gdk::Monitor>
HeadInfo::get_monitor() const
{
  return monitor;
}

//! Returns the display monitor index of this head, or -1 when it is unknown.
#if GTK_CHECK_VERSION(4, 0, 0)
int
HeadInfo::get_monitor_index(const Glib::RefPtr<Gdk::Display> &display) const
{
  if (!display || !get_monitor())
    {
      return -1;
    }

  auto monitors = display->get_monitors();
  guint num_monitors = monitors->get_n_items();

  for (guint i = 0; i < num_monitors; ++i)
    {
      auto m = std::dynamic_pointer_cast<Gdk::Monitor>(monitors->get_object(i));
      if (m && m->gobj() == get_monitor()->gobj())
        {
          return static_cast<int>(i);
        }
    }
  return -1;
}
#else
int
HeadInfo::get_monitor_index(const Glib::RefPtr<Gdk::Screen> screen) const
{
  if (!screen || !get_monitor())
    {
      return -1;
    }

  auto display = screen->get_display();
  if (!display)
    {
      return -1;
    }

  int num_monitors = display->get_n_monitors();

  for (int i = 0; i < num_monitors; ++i)
    {
      auto m = display->get_monitor(i);
      if (m && m->gobj() == get_monitor()->gobj())
        {
          return i;
        }
    }
  return -1;
}
#endif
