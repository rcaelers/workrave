// OSXInputMonitor.cc --- ActivityMonitor for OSX
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2012, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include "OSXInputMonitor.hh"

#include <unistd.h>

#include "debug.hh"
#include "input-monitor/IInputMonitorListener.hh"

OSXInputMonitor::OSXInputMonitor()
  : terminate_loop(false)
{
}


OSXInputMonitor::~OSXInputMonitor()
{
  if (monitor_thread)
    {
      monitor_thread->join();
    }
}


bool
OSXInputMonitor::init()
{
  mach_port_t master;
  IOMasterPort(MACH_PORT_NULL, &master);
  io_service = IOServiceGetMatchingService(master,
                                           IOServiceMatching("IOHIDSystem"));

  monitor_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&OSXInputMonitor::run, this)));
  return true;
}


//! Stops the activity monitoring.
void
OSXInputMonitor::terminate()
{
  terminate_loop = true;
  monitor_thread->join();
}


void
OSXInputMonitor::run()
{
  TRACE_ENTER("OSXInputMonitor::run");

  while (!terminate_loop)
    {
      CFTypeRef property;
      uint64_t idle_time = 0;

      property = IORegistryEntryCreateCFProperty(io_service,
                                                 CFSTR("HIDIdleTime"),
                                                 kCFAllocatorDefault,
                                                 0);
      CFNumberGetValue(reinterpret_cast<CFNumberRef>(property),
                       kCFNumberSInt64Type, &idle_time);
      CFRelease(property);

      TRACE_MSG(idle_time);

      if (idle_time < 1000000000)
        {
          fire_action();
          TRACE_MSG("fire");
        }

      usleep(500000);
    }
  TRACE_EXIT()
}
