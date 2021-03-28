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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MacOSInputMonitorFactory.hh"
#include "MacOSInputMonitor.hh"

using namespace workrave::input_monitor;

MacOSInputMonitorFactory::MacOSInputMonitorFactory(workrave::config::IConfigurator::Ptr config)
{
  (void)config;
  monitor = nullptr;
}

void
MacOSInputMonitorFactory::init(const char *display)
{
  (void)display;
}

IInputMonitor::Ptr
MacOSInputMonitorFactory::create_monitor(MonitorCapability capability)
{
  (void)capability;

  if (monitor == nullptr)
    {
      monitor = std::make_shared<MacOSInputMonitor>();

      bool init_ok = monitor->init();
      if (!init_ok)
        {
          monitor.reset();
        }
    }

  return monitor;
}
