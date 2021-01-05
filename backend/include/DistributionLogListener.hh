// DistributionLogListener.hh
//
// Copyright (C) 2002, 2003, 2005, 2007 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef DISTRIBUTIONLOGLISTENER_HH
#define DISTRIBUTIONLOGLISTENER_HH

namespace workrave
{
  class DistributionLogListener
  {
  public:
    virtual ~DistributionLogListener() {}

    //! Notification that a new log message has arrived.
    virtual void distribution_log(string msg) = 0;
  };
} // namespace workrave

#endif // DISTRIBUTIONLOGLISTENER_HH
