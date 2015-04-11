// OSXInputMonitorFactory.hh --- Factory to create input monitors.
//
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

#ifndef OSXINPUTMONITORFACTORY_HH
#define OSXINPUTMONITORFACTORY_HH

#include <stdlib.h>
#include <string>

#include "input-monitor/IInputMonitor.hh"
#include "input-monitor/IInputMonitorFactory.hh"
#include "config/IConfigurator.hh"

//! Factory to create input monitors.
class OSXInputMonitorFactory : public workrave::input_monitor::IInputMonitorFactory
{
public:
  OSXInputMonitorFactory(workrave::config::IConfigurator::Ptr config);
  void init(const std::string &display) override;
  workrave::input_monitor::IInputMonitor::Ptr create_monitor(MonitorCapability capability) override;

private:
  workrave::input_monitor::IInputMonitor::Ptr monitor;
};

#endif // INPUTMONITORFACTORY_HH
