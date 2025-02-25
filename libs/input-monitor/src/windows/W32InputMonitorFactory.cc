// Copyright (C) 2007, 2008, 2010, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include "W32InputMonitorFactory.hh"
#include "W32InputMonitor.hh"
#include "W32LowLevelMonitor.hh"
#include "W32AlternateMonitor.hh"

using namespace std;
using namespace workrave;

W32InputMonitorFactory::W32InputMonitorFactory(IConfigurator::Ptr config)
  : config(config)
  , actual_monitor_method{"monitor.method", ""}
{
  activity_monitor = NULL;
  statistics_monitor = NULL;
}

void
W32InputMonitorFactory::init(const char *display)
{
  (void)display;
}

//! Retrieves the input activity monitor
IInputMonitor::Ptr
W32InputMonitorFactory::create_monitor(MonitorCapability capability)
{
  if (capability == MonitorCapability::Activity)
    {
      return create_activity_monitor();
    }
  else if (capability == MonitorCapability::Statistics)
    {
      return create_statistics_monitor();
    }

  return IInputMonitor::Ptr();
}

//! Retrieves the input activity monitor
IInputMonitor::Ptr
W32InputMonitorFactory::create_activity_monitor()
{
  TRACE_ENTRY();
  IInputMonitor::Ptr monitor = NULL;

  if (activity_monitor != NULL)
    {
      monitor = activity_monitor;
    }
  else
    {
      bool initialized = false;
      string configure_monitor_method;
      int max_tries = 3;

      config->get_value_with_default("advanced/monitor", configure_monitor_method, "default");

      if (configure_monitor_method == "default")
        {
          TRACE_MSG("use default");
          actual_monitor_method = "nohook";
        }
      else
        {
          TRACE_MSG("use configured: {}", configure_monitor_method);
          actual_monitor_method = configure_monitor_method;
        }

      while (!initialized && max_tries > 0)
        {
          TRACE_MSG("try: {}", actual_monitor_method);

          if (actual_monitor_method == "lowlevel")
            {
              monitor = IInputMonitor::Ptr(new W32LowLevelMonitor(config));
              initialized = monitor->init();

              if (!initialized)
                {
                  monitor.reset();

                  actual_monitor_method = "nohook";
                  TRACE_MSG("failed to init");
                }
            }

          else if (actual_monitor_method == "nohook")
            {
              monitor = IInputMonitor::Ptr(new W32AlternateMonitor(config));
              initialized = monitor->init();

              if (!initialized)
                {
                  monitor.reset();

                  actual_monitor_method = "normal";
                  TRACE_MSG("failed to init");
                }
            }

          else if (actual_monitor_method == "normal")
            {
#if defined(HAVE_HARPOON)
              monitor = IInputMonitor::Ptr(new W32InputMonitor(config));
              initialized = monitor->init();

              if (!initialized)
                {
                  monitor.reset();

                  actual_monitor_method = "lowlevel";
                  TRACE_MSG("failed to init");
                }
#else
              actual_monitor_method = "lowlevel";
              TRACE_MSG("normal not available");
#endif
            }

          max_tries--;
        }

      if (!initialized)
        {
          MessageBoxA(NULL,
                      "Workrave must be able to monitor certain system "
                      "events in order to determine when you are idle.\n\n"

                      "Attempts were made to monitor your system, "
                      "but they were unsuccessful.\n\n"

                      "Workrave has reset itself to use its default monitor."
                      "Please run Workrave again. If you see this message "
                      "again, please file a bug report:\n\n"

                      "http://issues.workrave.org/\n\n"

                      "Workrave must exit now.\n",
                      "Workrave",
                      MB_OK);

          config->set_value("advanced/monitor", "normal");
          config->save();

          actual_monitor_method = "";
        }
      else
        {
          activity_monitor = monitor;

          if (configure_monitor_method != "default")
            {
              config->set_value("advanced/monitor", actual_monitor_method);
              config->save();
            }

          TRACE_MSG("using {}", actual_monitor_method);
        }
    }

  return monitor;
}

//! Retrieves the current input monitor for detailed statistics
IInputMonitor::Ptr
W32InputMonitorFactory::create_statistics_monitor()
{
  TRACE_ENTRY();
  if (activity_monitor == NULL)
    {
      create_activity_monitor();
    }

  if (actual_monitor_method == "nohook")
    {
      IInputMonitor::Ptr monitor = IInputMonitor::Ptr(new W32LowLevelMonitor(config));
      bool initialized = monitor->init();

      if (!initialized)
        {
          TRACE_MSG("failed to init lowlevel monitor");
          monitor.reset();
        }
      else
        {
          TRACE_MSG("use lowlevel monitor");
          statistics_monitor = monitor;
          return statistics_monitor;
        }
    }
  else
    {
      TRACE_MSG("use activity monitor");
      return activity_monitor;
    }

  return IInputMonitor::Ptr();
}
