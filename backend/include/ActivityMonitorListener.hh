// ActivityMonitorListener.hh
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef ACTIVITYMONITORLISTENER_HH
#define ACTIVITYMONITORLISTENER_HH

class ActivityMonitorListener
{
public:
  // Notification that the user is currently active.
  virtual bool action_notify() = 0;
};

#endif // ACTIVITYMONITORLISTENERINTERFACE_HH
