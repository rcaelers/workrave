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

#include "input-monitor/IInputMonitorFactory.hh"
#include "utils/Diagnostics.hh"
#include "config/IConfigurator.hh"

//! Factory to create input monitors.
class UnixInputMonitorFactory : public workrave::input_monitor::IInputMonitorFactory
{
public:
  explicit UnixInputMonitorFactory(workrave::config::IConfigurator::Ptr config);

  void init(const char *display) override;
  workrave::input_monitor::IInputMonitor::Ptr create_monitor(workrave::input_monitor::MonitorCapability capability) override;

private:
  bool error_reported;
  TracedField<std::string> actual_monitor_method;
  workrave::input_monitor::IInputMonitor::Ptr monitor;
  const char *display{};
  workrave::config::IConfigurator::Ptr config;
};

#endif // UNIXINPUTMONITORFACTORY_HH
