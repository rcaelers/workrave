// OSXInputMonitor.cc --- ActivityMonitor for OSX
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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

static const char rcsid[] = "$Id: OSXInputMonitor.cc 1090 2006-10-01 20:49:47Z dotsphinx $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include "OSXInputMonitor.hh"
#include "IInputMonitorListener.hh"
#include "Thread.hh"

OSXInputMonitor::OSXInputMonitor()
  : terminate(false)
{
  monitor_thread = new Thread(this);
  io_service = NULL;
}


OSXInputMonitor::~OSXInputMonitor()
{
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
      delete monitor_thread;
    }
}


bool
OSXInputMonitor::init()
{
  mach_port_t master;
  IOMasterPort(MACH_PORT_NULL, &master);
  io_service = IOServiceGetMatchingService(master,
                                           IOServiceMatching("IOHIDSystem"));

  monitor_thread->start();
  return true;
}


//! Stops the activity monitoring.
void
OSXInputMonitor::terminate()
{  
  terminate = true;
  monitor_thread->wait();
}


void
OSXInputMonitor::run()
{
  TRACE_ENTER("OSXInputMonitor::run");

  while (!terminate)
    {
      CFTypeRef property;
      uint64_t idle_time = 0;

      property = IORegistryEntryCreateCFProperty(io_service,
                                                 CFSTR("HIDIdleTime"),
                                                 kCFAllocatorDefault,
                                                 0);
      CFNumberGetValue((CFNumberRef)property,
                       kCFNumberSInt64Type, &idle_time);
      CFRelease(property);

      if (idle_time < 1000000)
        {
          fire_action();
        }

      g_usleep(1000);
    }
  TRACE_EXIT()
}
