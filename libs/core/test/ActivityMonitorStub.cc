// Copyright (C) 2001 - 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "ActivityMonitorStub.hh"

void
ActivityMonitorStub::set_active(bool active)
{
  this->active = active;
  forced_idle = false;
}

void
ActivityMonitorStub::terminate()
{
}

void
ActivityMonitorStub::suspend()
{
  this->suspended = true;
}

void
ActivityMonitorStub::resume()
{
  this->suspended = false;
}

void
ActivityMonitorStub::force_idle()
{
  forced_idle = true;
  count = 0;
}

void
ActivityMonitorStub::set_listener(IActivityMonitorListener *l)
{
  listener = l;
}

ActivityState
ActivityMonitorStub::get_current_state()
{
  if (suspended)
    {
      return ACTIVITY_SUSPENDED;
    }
  if (active && !forced_idle)
    {
      return ACTIVITY_ACTIVE;
    }
  return ACTIVITY_IDLE;
}

void
ActivityMonitorStub::notify()
{
  IActivityMonitorListener *l = listener;
  if (l != nullptr)
    {
      if (!l->action_notify())
        {
          listener = nullptr;
        }
    }
}
