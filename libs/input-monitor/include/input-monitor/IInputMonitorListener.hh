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

#ifndef WORKRAVE_INPUT_MONITOR_INPUTMONITORLISTENER_HH
#define WORKRAVE_INPUT_MONITOR_INPUTMONITORLISTENER_HH

namespace workrave
{
  namespace input_monitor
  {
    //! Listener for events from the input monitor.
    class IInputMonitorListener
    {
    public:
      virtual ~IInputMonitorListener() = default;

      //! Generic user activity (if no details info is available)
      virtual void action_notify() = 0;

      //! Reports mouse movement activity
      virtual void mouse_notify(int x, int y, int wheel = 0) = 0;

      //! Reports mouse button activity
      virtual void button_notify(bool is_press) = 0;

      //! Reports keyboard activity
      virtual void keyboard_notify(bool repeat) = 0;
    };
  } // namespace input_monitor
} // namespace workrave

#endif // WORKRAVE_INPUT_MONITOR_IINPUTMONITORLISTENER_HH
