// ActivityMonitorListener.hh
//
// Copyright (C) 2001 - 2007 Rob Caelers & Raymond Penners
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

#ifndef ACTIVITYMONITORLISTENER_HH
#define ACTIVITYMONITORLISTENER_HH

//! Listener for user activity from the Activity Monitor
class ActivityMonitorListener
{
public:
  virtual ~ActivityMonitorListener() = default;

  // Notification that the user is currently active.
  virtual bool action_notify() = 0;
};

#endif // ACTIVITYMONITORLISTENER_HH
