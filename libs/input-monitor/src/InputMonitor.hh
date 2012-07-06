// InputMonitor.hh ---  Base class of an activity monitor
//
// Copyright (C) 2007, 2008, 2010, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef INPUTMONITOR_HH
#define INPUTMONITOR_HH

#include <stdlib.h>
#include <list>

#include "config/IConfigurator.hh"

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorListener.hh"

using namespace workrave::input_monitor;

//!  Base for activity monitors.
class InputMonitor
  : public IInputMonitor
{
public:
  virtual void subscribe(IInputMonitorListener *listener);
  virtual void unsubscribe(IInputMonitorListener *listener);

protected:
  void fire_action();
  void fire_mouse(int x, int y, int wheel = 0);
  void fire_button(bool is_press);
  void fire_keyboard(bool repeat);

private:
  //!
  std::list<IInputMonitorListener *> listeners;
};

#include "InputMonitor.icc"

#endif // INPUTMONITOR_HH
