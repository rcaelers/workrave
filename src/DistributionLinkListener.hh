// DistributionLinkListener.hh
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTIONLINKLISTENER_HH
#define DISTRIBUTIONLINKLISTENER_HH

class DistributionLinkListener
{
public:
  virtual void master_changed(bool result) = 0;
  virtual void state_transfer_complete() = 0;
  virtual void log(char *fmt, ...) = 0;
};

#endif // DISTRIBUTIONLINKLISTENER_HH
