//
// Copyright (C) 2001 - 2010, 2012 Rob Caelers <robc@krandor.nl>
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

#include "ActivityMonitorStub.hh"

ActivityMonitorStub::Ptr
ActivityMonitorStub::create()
{
  return Ptr(new ActivityMonitorStub());
}


ActivityMonitorStub::ActivityMonitorStub() :
  suspended(false),
  state(ACTIVITY_IDLE)
{
}


ActivityMonitorStub::~ActivityMonitorStub()
{
}


void
ActivityMonitorStub::terminate()
{
}


void
ActivityMonitorStub::suspend()
{
  suspended = true;
}


void
ActivityMonitorStub::resume()
{
  suspended = false;
}


ActivityState
ActivityMonitorStub::get_current_state()
{
  if (suspended)
    {
      return ACTIVITY_FORCED_IDLE;
    }

  return state;
}


void
ActivityMonitorStub::force_idle()
{
  state = ACTIVITY_FORCED_IDLE;
}


void
ActivityMonitorStub::set_listener(IActivityMonitorListener::Ptr l)
{
  (void)l;
}


void
ActivityMonitorStub::set_state(ActivityState s)
{
  state = s;
}
