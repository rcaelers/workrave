// UnixInputMonitorFactory.cc -- Factory to create input monitors
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string>
#include <vector>
#include <algorithm>

#include "debug.hh"

#include "core/ICore.hh"
#include "core/CoreFactory.hh"
#include "config/IConfigurator.hh"
#include "utils/StringUtil.hh"

#include "UnixInputMonitorFactory.hh"
#include "RecordInputMonitor.hh"
#include "X11InputMonitor.hh"
#include "XScreenSaverMonitor.hh"
#include "MutterInputMonitor.hh"

using namespace std;
using namespace workrave;

UnixInputMonitorFactory::UnixInputMonitorFactory()
  : actual_monitor_method{"monitor.method", ""}
{
}

void
UnixInputMonitorFactory::init(const char *display)
{
  this->display = display;
}

//! Retrieves the input activity monitor
IInputMonitor *
UnixInputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  TRACE_ENTER("UnixInputMonitorFactory::get_monitor");
  (void)capability;

  if (monitor == nullptr)
    {
      bool initialized = false;
      string configure_monitor_method;

      vector<string> available_monitors;
      StringUtil::split(HAVE_MONITORS, ',', available_monitors);

      TRACE_MSG("available_monitors " << HAVE_MONITORS << " " << available_monitors.size());

      CoreFactory::get_configurator()->get_value_with_default("advanced/monitor", configure_monitor_method, "default");

      vector<string>::const_iterator start = available_monitors.end();

      if (configure_monitor_method != "default")
        {
          TRACE_MSG("use configured: " << configure_monitor_method);
          start = find(available_monitors.begin(), available_monitors.end(), configure_monitor_method);
        }

      if (start == available_monitors.end())
        {
          start = available_monitors.begin();
          TRACE_MSG("Start first available");
        }

      vector<string>::const_iterator loop = start;
      string monitor_method;
      while (1)
        {
          monitor_method = *loop;
          TRACE_MSG("Test " << monitor_method);

          if (monitor_method == "record")
            {
              monitor = new RecordInputMonitor(display);
            }
          else if (monitor_method == "screensaver")
            {
              monitor = new XScreenSaverMonitor();
            }
          else if (monitor_method == "x11events")
            {
              monitor = new X11InputMonitor(display);
            }
          else if (monitor_method == "mutter")
            {
              monitor = new MutterInputMonitor();
            }

          initialized = monitor->init();

          if (initialized)
            {
              TRACE_MSG("Success");
              break;
            }

          delete monitor;
          monitor = nullptr;

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
              g_idle_add(static_report_failure, nullptr);
            }

          CoreFactory::get_configurator()->set_value("advanced/monitor", "default");
          CoreFactory::get_configurator()->save();

          monitor_method = "";
        }
      else
        {
          if (configure_monitor_method != "default")
            {
              CoreFactory::get_configurator()->set_value("advanced/monitor", actual_monitor_method);
              CoreFactory::get_configurator()->save();
            }

          TRACE_MSG("using " << monitor_method);
        }

      actual_monitor_method = monitor_method;
    }

  TRACE_EXIT();
  return monitor;
}

gboolean
UnixInputMonitorFactory::static_report_failure(void *data)
{
  (void)data;

  ICore *core = CoreFactory::get_core();
  core->post_event(CORE_EVENT_MONITOR_FAILURE);

  return FALSE;
}
