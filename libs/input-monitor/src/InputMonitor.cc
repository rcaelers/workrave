// InputMonitor.cc
//
// Copyright (C) 2007, 2008, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "InputMonitor.hh"

using namespace workrave::input_monitor;

void
InputMonitor::subscribe(IInputMonitorListener *listener)
{
  listeners.push_back(listener);
}

void
InputMonitor::unsubscribe(IInputMonitorListener *listener)
{
  listeners.remove(listener);
}

void
InputMonitor::fire_action()
{
  for (auto &l: listeners)
    {
      l->action_notify();
    }
}

void
InputMonitor::fire_mouse(int x, int y, int wheel)
{
  for (auto &l: listeners)
    {
      l->mouse_notify(x, y, wheel);
    }
}

void
InputMonitor::fire_button(bool is_press)
{
  for (auto &l: listeners)
    {
      l->button_notify(is_press);
    }
}

void
InputMonitor::fire_keyboard(bool repeat)
{
  for (auto &l: listeners)
    {
      l->keyboard_notify(repeat);
    }
}
