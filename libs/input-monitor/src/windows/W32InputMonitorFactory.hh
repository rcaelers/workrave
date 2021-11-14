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

#ifndef W32INPUTMONITORFACTORY_HH
#define W32INPUTMONITORFACTORY_HH

#include <stdlib.h>
#include <string>

#include "input-monitor/IInputMonitorFactory.hh"
#include "input-monitor/IInputMonitor.hh"
#include "utils/Diagnostics.hh"
#include "config/IConfigurator.hh"

using namespace workrave::config;
using namespace workrave::input_monitor;

//! Factory to create input monitors.
class W32InputMonitorFactory : public workrave::input_monitor::IInputMonitorFactory
{
public:
  W32InputMonitorFactory(workrave::config::IConfigurator::Ptr config);
  virtual void init(const char *display);
  virtual workrave::input_monitor::IInputMonitor::Ptr create_monitor(MonitorCapability capability);

private:
  workrave::input_monitor::IInputMonitor::Ptr create_statistics_monitor();
  workrave::input_monitor::IInputMonitor::Ptr create_activity_monitor();

private:
  workrave::config::IConfigurator::Ptr config;
  workrave::input_monitor::IInputMonitor::Ptr activity_monitor;
  workrave::input_monitor::IInputMonitor::Ptr statistics_monitor;
  TracedField<std::string> actual_monitor_method;
};

#endif // W32INPUTMONITORFACTORY_HH
