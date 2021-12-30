// Copyright (C) 2002 - 2020 Rob Caelers <robc@krandor.nl>
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

#include "MacOSInputMonitor.hh"

#include <ApplicationServices/ApplicationServices.h>

#include <unistd.h>

#include "debug.hh"
#include "input-monitor/IInputMonitorListener.hh"

MacOSInputMonitor::~MacOSInputMonitor()
{
  if (monitor_thread)
    {
      monitor_thread->join();
    }
}

bool
MacOSInputMonitor::init()
{
  monitor_thread = std::shared_ptr<std::thread>(new std::thread(std::bind(&MacOSInputMonitor::run, this)));
  return true;
}

void
MacOSInputMonitor::terminate()
{
  terminate_loop = true;
  monitor_thread->join();
}

uint64_t
MacOSInputMonitor::get_event_count()
{
  static const CGEventType events[] = {kCGEventFlagsChanged,
                                       kCGEventKeyDown,
                                       kCGEventKeyUp,
                                       kCGEventLeftMouseDown,
                                       kCGEventLeftMouseDragged,
                                       kCGEventLeftMouseUp,
                                       kCGEventMouseMoved,
                                       kCGEventOtherMouseDown,
                                       kCGEventOtherMouseDragged,
                                       kCGEventOtherMouseUp,
                                       kCGEventRightMouseDown,
                                       kCGEventRightMouseDragged,
                                       kCGEventRightMouseUp,
                                       kCGEventScrollWheel,
                                       kCGEventTabletPointer,
                                       kCGEventTabletProximity};
  uint64_t count = 0;
  for (auto event: events)
    {
      count += CGEventSourceCounterForEventType(kCGEventSourceStateCombinedSessionState, event);
    }
  return count;
}

void
MacOSInputMonitor::run()
{
  TRACE_ENTRY();
  while (!terminate_loop)
    {
      uint64_t event_count = get_event_count();

      if (last_event_count != event_count)
        {
          fire_action();
        }

      last_event_count = event_count;
      usleep(1000000);
    }
}
