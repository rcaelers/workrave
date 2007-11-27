// W32InputMonitorFactory.cc -- Factory to create input monitors
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

static const char rcsid[] = "$Id: CoreFactory.cc 1351 2007-10-14 20:56:54Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "debug.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "W32InputMonitorFactory.hh"
#include "W32InputMonitor.hh"
#include "W32LowLevelMonitor.hh"
#include "W32AlternateMonitor.hh"

IInputMonitor *W32InputMonitorFactory::activity_monitor = NULL;
IInputMonitor *W32InputMonitorFactory::statistics_monitor = NULL;
std::string W32InputMonitorFactory::actual_monitor_method = "";

using namespace std;
using namespace workrave;


void
W32InputMonitorFactory::init(const std::string &display)
{
  (void) display;
}


//! Retrieves the input activity monitor
IInputMonitor *
OSXInputMonitorFactory::get_monitor(IInputMonitorFactory::MonitorCapability capability)
{
  if (capability == CAPABILITY_ACTIVITY)
    {
      return create_activity_monitor();
    }
  else if (capability == CAPABILITY_ACTIVITY)
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
  
      CoreFactory::get_configurator()->get_value_with_default("advanced/monitor",
                                                              configure_monitor_method,
                                                              "default");

      if (configure_monitor_method == "default")
        {
          actual_monitor_method = "nohook";
        }
      else
        {
          actual_monitor_method = configure_monitor_method;
      
        }

      while (!initialized && max_tries > 0)
        {
          if (actual_monitor_method == "lowlevel")
            {
              monitor = new W32LowLevelMonitor();
              initialized = monitor->init();
          
              if (!initialized)
                {
                  delete monitor;
                  monitor = NULL;
              
                  actual_monitor_method = "nohook";
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
                }
            }

          max_tries--;
        }
  
      if (!initialized)
        {
          MessageBoxA( NULL,
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
                       MB_OK );
      
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
        }
    }
  
  TRACE_EXIT();
  return monitor;
}

//! Retrieves the current input monitor for detailed statistics
IInputMonitor *
W32InputMonitorFactory::create_statistics_monitor()
{
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
          delete monitor;
          monitor = NULL;
        }
      else
        {
          statistics_monitor = monitor;
          return statistics_monitor;
        }
    }
  else
    {
      return activity_monitor;
    }

  return NULL;
}
