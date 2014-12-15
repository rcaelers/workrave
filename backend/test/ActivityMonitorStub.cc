//
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
#include "config.h"
#endif

#include "ActivityMonitorStub.hh"

#include "utils/TimeSource.hh"

#include "debug.hh"

using namespace std;
using namespace workrave::utils;

ActivityMonitorStub::ActivityMonitorStub() : active(false), suspended(false), forced_idle(false)
{
}

ActivityMonitorStub::~ActivityMonitorStub()
{
}

void
ActivityMonitorStub::set_active(bool active)
{
  this->active = active;
  forced_idle = false;
}

void
ActivityMonitorStub::init()
{
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
}

bool
ActivityMonitorStub::is_active()
{
  return !suspended && !forced_idle && active;
}

void
ActivityMonitorStub::set_listener(IActivityMonitorListener::Ptr l)
{
  listener = l;
}

void
ActivityMonitorStub::notify()
{
  IActivityMonitorListener::Ptr l;

  l = listener;
  if (l)
    {
      if (!l->action_notify())
        {
          listener.reset();
        }
    }
}
