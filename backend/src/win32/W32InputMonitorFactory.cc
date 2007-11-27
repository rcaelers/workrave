// InputMonitorFactory.cc
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

#include "debug.hh"
#include <assert.h>
#include <string>

#include "W32InputMonitorFactory.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "W32InputMonitor.hh"
#include "W32LowLevelMonitor.hh"
#include "W32AlternateMonitor.hh"

using namespace std;
using namespace workrave;

W32InputMonitorFactory::W32InputMonitorFactory()
{
  monitor = NULL;
  detailed_monitor = NULL;
}


//! Creates the input monitor
IInputMonitor *
W32InputMonitorFactory::create_monitor(IInputMonitorListener *listener,
                                       const char *display)
{
  TRACE_ENTER("W32InputMonitorFactory::create_monitor");

  (void) display;
  
  bool initialized = false;
  string monitor_method;
  
  CoreFactory::get_configurator()->get_value_with_default("advanced/monitor", monitor_method, "normal");
  
  if (monitor_method == "lowlevel")
    {
      monitor = new W32LowLevelMonitor();
      initialized = monitor->init(listener);
      
      if (!initialized)
        {
          int ret = MessageBoxA( NULL, 
             "Workrave's alternate low-level activity monitor failed to initialize.\n\n"
             "Click 'OK' to try using the regular activity monitor instead.\n",
             "Workrave: Debug Message", 
             MB_OKCANCEL | MB_ICONERROR | MB_TOPMOST );
              
          delete monitor;
          monitor = NULL;
          if( ret != IDOK )
            {
              delete CoreFactory::get_configurator();
              TRACE_EXIT();
              exit(0);
            }
          
          monitor_method = "normal";
          CoreFactory::get_configurator()->set_value("advanced/monitor", monitor_method);
          CoreFactory::get_configurator()->save();
        }
    }
  

  if (monitor_method == "nohook")
    {
      monitor = new W32AlternateMonitor();
      initialized = monitor->init(listener);

      if (!initialized)
        {
          int ret = MessageBoxA( NULL, 
             "Workrave's alternate activity monitor failed to initialize.\n\n"
             "Click 'OK' to try using the regular activity monitor instead.\n",
             "Workrave: Debug Message", 
             MB_OKCANCEL | MB_ICONERROR | MB_TOPMOST );
              
          delete monitor;
          monitor = NULL;
          if( ret != IDOK )
            {
              delete CoreFactory::get_configurator();
              TRACE_EXIT();
              exit(0);
            }
          
          monitor_method = "normal";
          CoreFactory::get_configurator()->set_value("advanced/monitor", monitor_method);
          CoreFactory::get_configurator()->save();
        }
    }

  if (monitor_method == "normal" || !initialized)
    {
      monitor = new W32InputMonitor();
      initialized = monitor->init(listener);

      if (!initialized)
        {
          int ret = MessageBoxA( NULL,
              "Workrave must be able to monitor certain system "
              "events in order to determine when you are idle.\n\n"

              "An attempt was made to hook into your system, but it "
              "was unsuccessful.\n\n"

              "You might have system safety software that blocks "
              "Workrave from installing global hooks on your system.\n\n"

              "You can instead run Workrave using the alternate monitor, "
              "which doesn't require global hooks.\n\n"

              "Click 'OK' to run the alternate monitor, or 'Cancel' to exit.\n",

              "Workrave: Debug Message",
              MB_OKCANCEL | MB_ICONSTOP | MB_TOPMOST );

          /*
          We want the current input monitor destructor called
          at this point, after the user responds to the messagebox.
          This way, if debugging, debug output can be viewed before
          the user responds to the message box.
          */
          delete monitor;
          monitor = NULL;

          if( ret == IDCANCEL )
            {
              delete CoreFactory::get_configurator();
              TRACE_EXIT();
              exit( 0 );
            }
          else
            {
              monitor_method = "nohook";
              CoreFactory::get_configurator()->set_value("advanced/monitor", monitor_method);
              CoreFactory::get_configurator()->save();
            }
        }
    }

  // Final try:
  if (monitor_method == "nohook" && !initialized)
    {
      monitor = new W32AlternateMonitor();
      initialized = monitor->init(listener);

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
                "Workrave: Debug Message",
                MB_OK );
          
          delete monitor;
          monitor = NULL;
          
          monitor_method = "normal";
          CoreFactory::get_configurator()->set_value("advanced/monitor", monitor_method);
          CoreFactory::get_configurator()->save();
          
          delete CoreFactory::get_configurator();
          
          TRACE_EXIT();
          exit( 0 );
        }
    }

  return monitor;
}


//! Creates the details input monitor
IInputMonitor *
W32InputMonitorFactory::create_detailed_monitor(IInputMonitorListener *listener,
                                                const char *display)
{
  (void) listener;
  (void) display;

  return NULL;
}
