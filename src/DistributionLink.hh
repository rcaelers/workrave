// DistributionLink.hh
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

#ifndef DISTRIBUTIONLINK_HH
#define DISTRIBUTIONLINK_HH

#include <string>

class DistributionLinkListener;
class DistributedStateInterface;

#include "DistributedStateInterface.hh"

class DistributionLink
{
public:
  DistributionLink() {}
  virtual ~DistributionLink() {}

  virtual int get_number_of_peers() = 0;
  virtual void set_distribution_manager(DistributionLinkListener *dll) = 0;
  virtual bool set_enabled(bool enabled) = 0;
  virtual bool init() = 0;
  virtual void heartbeat() = 0;
  virtual void set_user(string username, string password) = 0;
  virtual void join(string url) = 0;
  virtual bool claim() = 0;
  virtual bool register_state(DistributedStateID id, DistributedStateInterface *dist_state) = 0;
  virtual bool unregister_state(DistributedStateID id) = 0;
  virtual bool disconnect_all() = 0;
  virtual bool reconnect_all() = 0;
};

#endif // DISTRIBUTIONLINK_HH
