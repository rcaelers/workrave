// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_INPUT_MONITOR_IINPUTMONITOR_HH
#define WORKRAVE_INPUT_MONITOR_IINPUTMONITOR_HH

#include <memory>

namespace workrave
{
  namespace input_monitor
  {
    class IInputMonitorListener;

    //! Interface that all input monitors must support.
    class IInputMonitor
    {
    public:
      using Ptr = std::shared_ptr<IInputMonitor>;

      virtual ~IInputMonitor() = default;

      //! Initializes the activity monitor.
      virtual bool init() = 0;

      //! Stops the activity monitoring.
      virtual void terminate() = 0;

      //! Subscribe for activity monitor.
      virtual void subscribe(IInputMonitorListener *listener) = 0;

      //! Unsubscribe for activity monitor.
      virtual void unsubscribe(IInputMonitorListener *listener) = 0;
    };
  } // namespace input_monitor
} // namespace workrave

#endif // WORKRAVE_INPUT_MONITOR_IINPUTMONITOR_HH
