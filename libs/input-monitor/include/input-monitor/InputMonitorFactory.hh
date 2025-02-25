// Copyright (C) 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_INPUT_MONITOR_INPUTMONITORFACTORY_HH
#define WORKRAVE_INPUT_MONITOR_INPUTMONITORFACTORY_HH

#include "config/IConfigurator.hh"
#include "input-monitor/IInputMonitorFactory.hh"
#include "input-monitor/IInputMonitor.hh"

namespace workrave
{
  namespace input_monitor
  {
    //! Factory to create input monitors.
    class InputMonitorFactory
    {
    public:
      static void init(workrave::config::IConfigurator::Ptr config, const char *display);
      static workrave::input_monitor::IInputMonitor::Ptr create_monitor(workrave::input_monitor::MonitorCapability capability);

    private:
      static workrave::input_monitor::IInputMonitorFactory *factory;
    };
  } // namespace input_monitor
} // namespace workrave

#endif // WORKRAVE_INPUT_MONITOR_INPUTMONITORFACTORY_HH
