// DistributionLogListener.hh
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTIONLOGLISTENER_HH
#define DISTRIBUTIONLOGLISTENER_HH

class DistributionLogListener
{
public:
  //! Notification that a new log message has arrived.
  virtual void distribution_log(string msg) = 0;
};

#endif // DISTRIBUTIONLOGLISTENER_HH
