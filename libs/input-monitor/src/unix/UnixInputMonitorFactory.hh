// UnixInputMonitorFactory.hh --- Factory to create input monitors.
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

#ifndef UNIXINPUTMONITORFACTORY_HH
#define UNIXINPUTMONITORFACTORY_HH

#include <stdlib.h>
#include <string>

#include "input-monitor/IInputMonitorFactory.hh"

using namespace workrave::input_monitor;

//! Factory to create input monitors.
class UnixInputMonitorFactory  : public IInputMonitorFactory
{
public:
  UnixInputMonitorFactory(workrave::config::IConfigurator::Ptr config);

  virtual void init(const std::string &display);
  virtual IInputMonitor::Ptr create_monitor(IInputMonitorFactory::MonitorCapability capability);

  bool error_reported;
  std::string actual_monitor_method;
  IInputMonitor::Ptr monitor;
  std::string display;
  workrave::config::IConfigurator::Ptr config;
};

#endif // UNIXINPUTMONITORFACTORY_HH
