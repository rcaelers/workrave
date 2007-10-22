// InputMonitor.cc
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

static const char rcsid[] = "$Id: Core.cc 1351 2007-10-14 20:56:54Z rcaelers $";

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
