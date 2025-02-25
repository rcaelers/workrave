// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef IACTIVITYMONITOR_HH
#define IACTIVITYMONITOR_HH

#include "config/Config.hh"

class IActivityMonitorListener
{
public:
  using Ptr = std::shared_ptr<IActivityMonitorListener>;

  virtual ~IActivityMonitorListener() = default;

  // Notification that the user is currently active.
  virtual bool action_notify() = 0;
};

class IActivityMonitor
{
public:
  using Ptr = std::shared_ptr<IActivityMonitor>;

public:
  virtual ~IActivityMonitor() = default;

  virtual void init() = 0;
  virtual void terminate() = 0;
  virtual void suspend() = 0;
  virtual void resume() = 0;
  virtual void force_idle() = 0;
  virtual bool is_active() = 0;
  virtual void set_listener(IActivityMonitorListener::Ptr l) = 0;
};

#endif // IACTIVITYMONITOR_HH
