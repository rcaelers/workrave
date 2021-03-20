// InputMonitor.hh ---  Base class of an activity monitor
//
// Copyright (C) 2007, 2008, 2010 Rob Caelers <robc@krandor.nl>
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

#include "IInputMonitor.hh"
#include "IInputMonitorListener.hh"
#include <cstdlib>

// Forward declarion of internal interfaces.
class IInputMonitorListener;

//!  Base for activity monitors.
class InputMonitor : public IInputMonitor
{
public:
  InputMonitor() = default;
  ~InputMonitor() override = default;

  void subscribe_activity(IInputMonitorListener *listener) override;
  void subscribe_statistics(IInputMonitorListener *listener) override;
  void unsubscribe_activity(IInputMonitorListener *listener) override;
  void unsubscribe_statistics(IInputMonitorListener *listener) override;

protected:
  void fire_action();
  void fire_mouse(int x, int y, int wheel = 0);
  void fire_button(bool is_press);
  void fire_keyboard(bool repeat);

private:
  //!
  IInputMonitorListener *activity_listener{nullptr};

  //!
  IInputMonitorListener *statistics_listener{nullptr};
};

#include "InputMonitor.icc"

#endif // INPUTMONITOR_HH
