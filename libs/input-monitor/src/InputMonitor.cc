// InputMonitor.cc
//
// Copyright (C) 2007, 2008, 2012 Rob Caelers <robc@krandor.nl>
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

#include <assert.h>

#include "InputMonitor.hh"


InputMonitor::InputMonitor()
  : activity_listener(NULL),
    statistics_listener(NULL)
{
}


InputMonitor::~InputMonitor()
{
}


void
InputMonitor::subscribe_activity(IInputMonitorListener *listener)
{
  assert(activity_listener == NULL);
  activity_listener = listener;
}


void
InputMonitor::subscribe_statistics(IInputMonitorListener *listener)
{
  assert(statistics_listener == NULL);
  statistics_listener = listener;
}


void
InputMonitor::unsubscribe_activity(IInputMonitorListener *listener)
{
  (void) listener;
  assert(activity_listener != NULL);
  activity_listener = NULL;
}


void
InputMonitor::unsubscribe_statistics(IInputMonitorListener *listener)
{
  (void) listener;
  assert(statistics_listener != NULL);
  statistics_listener = NULL;
}
