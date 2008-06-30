// InputMonitor.hh ---  Base class of an activity monitor
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: IInputMonitor.hh 1351 2007-10-14 20:56:54Z rcaelers $
//

#ifndef INPUTMONITOR_HH
#define INPUTMONITOR_HH

#include <stdlib.h>
#include "IInputMonitor.hh"
#include "IInputMonitorListener.hh"

// Forward declarion of internal interfaces.
class IInputMonitorListener;

//!  Base for activity monitors.
class InputMonitor
  : public IInputMonitor
{
public:
  InputMonitor();
  virtual ~InputMonitor();

  virtual void subscribe_activity(IInputMonitorListener *listener);
  virtual void subscribe_statistics(IInputMonitorListener *listener);
  virtual void unsubscribe_activity(IInputMonitorListener *listener);
  virtual void unsubscribe_statistics(IInputMonitorListener *listener);

protected:
  void fire_action();
  void fire_mouse(int x, int y, int wheel = 0);
  void fire_button(int button_mask, bool is_press);
  void fire_keyboard(int key_code, int modifier);

private:
  //!
  IInputMonitorListener *activity_listener;

  //!
  IInputMonitorListener *statistics_listener;
};

#include "InputMonitor.icc"

#endif // INPUTMONITOR_HH
