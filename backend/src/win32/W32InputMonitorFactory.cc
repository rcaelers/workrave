// W32InputMonitorFactory.cc -- Factory to create input monitors
//
// Copyright (C) 2007, 2008, 2010, 2011 Rob Caelers <robc@krandor.nl>
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

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "W32InputMonitorFactory.hh"
#include "W32InputMonitor.hh"
#include "W32LowLevelMonitor.hh"
#include "W32AlternateMonitor.hh"

using namespace std;
using namespace workrave;

W32InputMonitorFactory::W32InputMonitorFactory()
  : actual_monitor_method{"monitor.method", ""}
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
IInputMonitor *
W32InputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  if (capability == CAPABILITY_ACTIVITY)
    {
      return create_activity_monitor();
    }
  else if (capability == CAPABILITY_STATISTICS)
    {
      return create_statistics_monitor();
    }

  return NULL;
}

//! Retrieves the input activity monitor
IInputMonitor *
W32InputMonitorFactory::create_activity_monitor()
{
  TRACE_ENTER("W32InputMonitorFactory::create_activity_monitor");
  IInputMonitor *monitor = NULL;

  if (activity_monitor != NULL)
    {
      monitor = activity_monitor;
    }
  else
    {
      bool initialized = false;
      string configure_monitor_method;
      int max_tries = 3;

      CoreFactory::get_configurator()->get_value_with_default("advanced/monitor", configure_monitor_method, "default");

      if (configure_monitor_method == "default")
        {
          TRACE_MSG("use default");
          actual_monitor_method = "nohook";
        }
      else
        {
          TRACE_MSG("use configured: " << configure_monitor_method);
          actual_monitor_method = configure_monitor_method;
        }

      while (!initialized && max_tries > 0)
        {
          TRACE_MSG("try: " << actual_monitor_method);

          if (actual_monitor_method == "lowlevel")
            {
              monitor = new W32LowLevelMonitor();
              initialized = monitor->init();

              if (!initialized)
                {
                  delete monitor;
                  monitor = NULL;

                  actual_monitor_method = "nohook";
                  TRACE_MSG("failed to init");
                }
            }

          else if (actual_monitor_method == "nohook")
            {
              monitor = new W32AlternateMonitor();
              initialized = monitor->init();

              if (!initialized)
                {
                  delete monitor;
                  monitor = NULL;

                  actual_monitor_method = "normal";
                  TRACE_MSG("failed to init");
                }
            }

          else if (actual_monitor_method == "normal")
            {
              monitor = new W32InputMonitor();
              initialized = monitor->init();

              if (!initialized)
                {
                  delete monitor;
                  monitor = NULL;

                  actual_monitor_method = "lowlevel";
                  TRACE_MSG("failed to init");
                }
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

          CoreFactory::get_configurator()->set_value("advanced/monitor", "normal");
          CoreFactory::get_configurator()->save();

          actual_monitor_method = "";
        }
      else
        {
          activity_monitor = monitor;

          if (configure_monitor_method != "default")
            {
              CoreFactory::get_configurator()->set_value("advanced/monitor", actual_monitor_method);
              CoreFactory::get_configurator()->save();
            }

          TRACE_MSG("using " << actual_monitor_method);
        }
    }

  TRACE_EXIT();
  return monitor;
}

//! Retrieves the current input monitor for detailed statistics
IInputMonitor *
W32InputMonitorFactory::create_statistics_monitor()
{
  TRACE_ENTER("W32InputMonitorFactory::create_statistics_monitor");
  if (activity_monitor == NULL)
    {
      create_activity_monitor();
    }

  if (actual_monitor_method == "nohook")
    {
      IInputMonitor *monitor = new W32LowLevelMonitor();
      bool initialized = monitor->init();

      if (!initialized)
        {
          TRACE_RETURN("failed to init lowlevel monitor");
          delete monitor;
          monitor = NULL;
        }
      else
        {
          TRACE_RETURN("use lowlevel monitor");
          statistics_monitor = monitor;
          return statistics_monitor;
        }
    }
  else
    {
      TRACE_RETURN("use activity monitor");
      return activity_monitor;
    }

  TRACE_EXIT();
  return NULL;
}
