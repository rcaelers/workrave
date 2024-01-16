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

#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "debug.hh"

#include "config/IConfigurator.hh"

#include "UnixInputMonitorFactory.hh"
#include "RecordInputMonitor.hh"
#include "X11InputMonitor.hh"
#include "XScreenSaverMonitor.hh"
#include "MutterInputMonitor.hh"
#include "WaylandInputMonitor.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;
using namespace workrave::input_monitor;

UnixInputMonitorFactory::UnixInputMonitorFactory(IConfigurator::Ptr config)
  : error_reported(false)
  , actual_monitor_method{"monitor.method", ""}
  , config(std::move(config))
{
  monitor = nullptr;
}

void
UnixInputMonitorFactory::init(const char *display)
{
  this->display = display;
}

//! Retrieves the input activity monitor
IInputMonitor::Ptr
UnixInputMonitorFactory::create_monitor(MonitorCapability capability)
{
  TRACE_ENTRY();
  (void)capability;

  if (monitor == nullptr)
    {
      bool initialized = false;
      string configure_monitor_method;

      vector<string> available_monitors;
      boost::split(available_monitors, HAVE_MONITORS, boost::is_any_of(","));

      TRACE_MSG("available_monitors {} {}", HAVE_MONITORS, available_monitors.size());

      config->get_value_with_default("advanced/monitor", configure_monitor_method, "default");

      auto start = available_monitors.end();

      if (configure_monitor_method != "default")
        {
          TRACE_MSG("use configured: {}", configure_monitor_method);
          start = find(available_monitors.begin(), available_monitors.end(), configure_monitor_method);
        }

      if (start == available_monitors.end())
        {
          start = available_monitors.begin();
          TRACE_MSG("Start first available");
        }

      string monitor_method;
      auto loop = start;
      while (true)
        {
          monitor_method = *loop;
          TRACE_MSG("Test {}", monitor_method);

          if (monitor_method == "record")
            {
              monitor = IInputMonitor::Ptr(new RecordInputMonitor(display));
            }
          else if (monitor_method == "screensaver")
            {
              monitor = IInputMonitor::Ptr(new XScreenSaverMonitor());
            }
          else if (monitor_method == "x11events")
            {
              monitor = IInputMonitor::Ptr(new X11InputMonitor(display));
            }
          else if (monitor_method == "mutter")
            {
              monitor = IInputMonitor::Ptr(new MutterInputMonitor());
            }
          else if (monitor_method == "wayland")
            {
              monitor = IInputMonitor::Ptr(new WaylandInputMonitor());
            }

          initialized = monitor->init();

          if (initialized)
            {
              TRACE_MSG("Success");
              break;
            }

          monitor.reset();

          loop++;
          if (loop == available_monitors.end())
            {
              loop = available_monitors.begin();
            }

          if (loop == start)
            {
              break;
            }
        }

      if (!initialized)
        {
          TRACE_MSG("Non found");

          if (!error_reported)
            {
              error_reported = true;
              // TODO: report failure.
              // g_idle_add(static_report_failure, NULL);
            }

          config->set_value("advanced/monitor", "default");
          config->save();

          monitor_method = "";
        }
      else
        {
          if (configure_monitor_method != "default")
            {
              config->set_value("advanced/monitor", actual_monitor_method);
              config->save();
            }

          TRACE_MSG("using {}", monitor_method);
        }
      actual_monitor_method = monitor_method;
    }

  return monitor;
}

// gboolean
// UnixInputMonitorFactory::static_report_failure(void *data)
// {
//   (void)data;

//   ICore *core = CoreFactory::get_core();
//   core->post_event(CORE_EVENT_MONITOR_FAILURE);

//   return FALSE;
// }
