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

#ifndef DISTRIBUTIOLINKLISTENER_HH
#define DISTRIBUTIOLINKLISTENER_HH

class DistributionLinkListener
{
public:
  virtual void active_changed(bool result) = 0;
};

#endif // DISTRIBUTIOLINKLISTENER_HH
