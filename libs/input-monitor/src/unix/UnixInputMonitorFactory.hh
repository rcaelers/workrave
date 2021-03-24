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

#include <cstdlib>
#include <string>

#include "input-monitor/IInputMonitorFactory.hh"
#include "utils/Diagnostics.hh"

#include <glib.h>

//! Factory to create input monitors.
class UnixInputMonitorFactory : public IInputMonitorFactory
{
public:
  UnixInputMonitorFactory();

  void init(const char *display) override;
  IInputMonitor *get_monitor(IInputMonitorFactory::MonitorCapability capability) override;

private:
  static gboolean static_report_failure(void *data);

  bool error_reported{false};
  TracedField<std::string> actual_monitor_method;
  IInputMonitor *monitor{nullptr};
  const char *display{nullptr};
};

#endif // UNIXINPUTMONITORFACTORY_HH
