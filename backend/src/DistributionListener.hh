// DistributionListener.hh
//
// Copyright (C) 2002, 2003, 2005 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTIONLISTENER_HH
#define DISTRIBUTIONLISTENER_HH

class DistributionListener
{
public:
  virtual ~DistributionListener() {}
  
  //! A remote client has signed on.
  virtual void signon_remote_client(string client_id) = 0;

  //! A remote client has signed off.
  virtual void signoff_remote_client(string client_id) = 0;
};

#endif // DISTRIBUTIONLISTENER_HH
