// UnixInputMonitorFactory.cc -- Factory to create input monitors
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "UnixInputMonitorFactory.hh"
#include "X11InputMonitor.hh"

UnixInputMonitorFactory::UnixInputMonitorFactory()
{
  monitor = NULL;
}

void
UnixInputMonitorFactory::init(const std::string &display)
{
  this->display = display;
}

//! Retrieves the input activity monitor
IInputMonitor *
UnixInputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  (void) capability;
  
  if (monitor == NULL)
    {
      monitor = new X11InputMonitor(display);

      bool init_ok = monitor->init();
      if (!init_ok)
        {
          delete monitor;
          monitor = NULL;
        }
    }

  return monitor;
}

