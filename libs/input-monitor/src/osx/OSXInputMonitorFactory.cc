// OSXInputMonitorFactory.cc -- Factory to create input monitors
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "debug.hh"

#include "OSXInputMonitorFactory.hh"
#include "OSXInputMonitor.hh"

using namespace workrave::input_monitor;

OSXInputMonitorFactory::OSXInputMonitorFactory(workrave::config::IConfigurator::Ptr config)
{
  (void) config;
  monitor = nullptr;
}

void
OSXInputMonitorFactory::init(const std::string &display)
{
  (void) display;
}


//! Retrieves the input activity monitor
IInputMonitor::Ptr
OSXInputMonitorFactory::create_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  (void) capability;

  if (monitor == nullptr)
    {
      monitor = IInputMonitor::Ptr(new OSXInputMonitor());

      bool init_ok = monitor->init();
      if (!init_ok)
        {
          monitor.reset();
        }
    }

  return monitor;
}
