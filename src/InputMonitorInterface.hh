// InputMonitorInterface.hh --- Interface definition for the Activity Monitor
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef INPUTMONITORINTERFACE_HH
#define INPUTMONITORINTERFACE_HH

class InputMonitorListenerInterface;

//! Interface that all activity monitors must support.
class InputMonitorInterface
{
public:
  //! Initializes the activity monitor.
  virtual void init(InputMonitorListenerInterface *) = 0;

  //! Stops the activity monitoring.
  virtual void terminate() = 0;
};

#endif // INPUTMONITORINTERFACE_HH
