// DistributionManager.hh
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

#ifndef DISTRIBUTIOMANAGER_HH
#define DISTRIBUTIOMANAGER_HH

#include <string>

#include "DistributionLinkListener.hh"
#include "DistributedStateInterface.hh"

class DistributionLink;

class DistributionManager : public DistributionLinkListener
{
public:
  enum NodeState { NODE_ACTIVE, NODE_PASSIVE, NODE_STANDBY };
  
  static DistributionManager *get_instance();
  
  DistributionManager();
  virtual ~DistributionManager();

  NodeState get_state() const;
  bool is_active() const;
  int get_number_of_peers();
  bool claim();
  bool join(string url);
  bool register_state(DistributedStateID id, DistributedStateInterface *dist_state);
  bool unregister_state(DistributedStateID id);

  //
  virtual void active_changed(bool result);
  
private:
  //! The one and only instance
  static DistributionManager *instance;

  //! Link to other clients
  DistributionLink *link;

  //! State
  NodeState state;
};



inline DistributionManager *
DistributionManager::get_instance()
{
  if (instance == NULL)
    {
      instance = new DistributionManager;
    }
  return instance;
}

#endif // DISTRIBUTIOMANAGER_HH
