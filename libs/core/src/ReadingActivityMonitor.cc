// Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef FAKEACTIVITYMONITOR_HH
#define FAKEACTIVITYMONITOR_HH

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "IActivityMonitor.hh"

class FakeActivityMonitor : public IActivityMonitor
{
public:
  FakeActivityMonitor() = default;
  ~FakeActivityMonitor() override = default;

  //! Stops the activity monitoring.
  void terminate() override
  {
  }

  //! Suspends the activity monitoring.
  void suspend() override
  {
    suspended = true;
  }

  //! Resumes the activity monitoring.
  void resume() override
  {
    suspended = false;
  }

  //! Returns the current state
  ActivityState get_current_state() override
  {
    if (suspended)
      {
        return ACTIVITY_SUSPENDED;
      }

    return state;
  }

  //! Force state to be idle.
  void force_idle() override
  {
    state = ACTIVITY_IDLE;
  }

  void set_state(ActivityState s)
  {
    state = s;
  }

  void set_listener(IActivityMonitorListener *l) override
  {
    (void)l;
  }

private:
  //! Monitor suspended?
  bool suspended{false};

  //! Current state
  ActivityState state{ACTIVITY_IDLE};
};

#endif // FAKEACTIVITYMONITOR_HH
