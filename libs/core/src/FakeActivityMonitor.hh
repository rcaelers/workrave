// Copyright (C) 2001 - 2007 Rob Caelers <robc@krandor.nl>
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

#include "IActivityMonitor.hh"

using namespace workrave;

class FakeActivityMonitor : public IActivityMonitor
{
public:
  FakeActivityMonitor()
    : suspended(false)
    , state(ACTIVITY_IDLE)
  {
  }

  virtual ~FakeActivityMonitor()
  {
  }

  //! Stops the activity monitoring.
  void terminate()
  {
  }

  //! Suspends the activity monitoring.
  void suspend()
  {
    suspended = true;
  }

  //! Resumes the activity monitoring.
  void resume()
  {
    suspended = false;
  }

  //! Returns the current state
  ActivityState get_current_state()
  {
    if (suspended)
      {
        return ACTIVITY_SUSPENDED;
      }

    return state;
  }

  //! Force state to be idle.
  void force_idle()
  {
    state = ACTIVITY_IDLE;
  }

  void set_state(ActivityState s)
  {
    state = s;
  }

  void set_listener(IActivityMonitorListener *l)
  {
    (void)l;
  }

private:
  //! Monitor suspended?
  bool suspended;

  //! Current state
  ActivityState state;
};

#endif // FAKEACTIVITYMONITOR_HH
