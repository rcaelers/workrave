// UnixInputMonitorFactory.hh --- Factory to create input monitors.
//
// Copyright (C) 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#include "IInputMonitorFactory.hh"

#include <glib.h>

//! Factory to create input monitors.
class UnixInputMonitorFactory  : public IInputMonitorFactory
{
public:
  UnixInputMonitorFactory();

  virtual void init(const std::string &display);
  virtual IInputMonitor *get_monitor(IInputMonitorFactory::MonitorCapability capability);

private:
  static gboolean static_report_failure(void *data);

  bool error_reported;
  std::string actual_monitor_method;
  IInputMonitor *monitor;
  std::string display;
};

#endif // UNIXINPUTMONITORFACTORY_HH
