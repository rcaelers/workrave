// DistributionManager.cc
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"

DistributionManager *DistributionManager::instance = NULL;


DistributionManager::DistributionManager()
  : link(NULL),
    state(NODE_STANDBY)
{
  link = new DistributionSocketLink();
  link->set_distribution_manager(this);
    
  const char *port = getenv("WORKRAVE_PORT");
  if (port != NULL)
    {
      link->init(atoi(port));

      const char *env = getenv("WORKRAVE_URL");
      if (env != NULL)
        {
          join(env);
        }
    }
}


DistributionManager::~DistributionManager()
{
}


DistributionManager::NodeState
DistributionManager::get_state() const
{
  return state;
}


int
DistributionManager::get_number_of_peers()
{
  int ret = 0;
  
  if (link != NULL)
    {
      ret = link->get_number_of_peers();
    }

  return ret;
}

bool
DistributionManager::is_active() const
{
  return state == NODE_ACTIVE;
}


bool
DistributionManager::claim()
{
  bool ret = false;
  
  if (link != NULL)
    {
      ret = link->claim();
    }

  if (ret)
    {
      state = NODE_ACTIVE;
    }
  
  return ret;
}


bool
DistributionManager::join(string url)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->join(url);
      ret = true;
    }
  return ret;
}


bool
DistributionManager::register_state(DistributesStateID id, DistributedStateInterface *dist_state)
{
  return false;
}


void
DistributionManager::unregister_state(int state_id)
{
}
  

void
DistributionManager::active_changed(bool result)
{
  TRACE_ENTER("DistributionManager::active_changed");
  state = (result ? NODE_ACTIVE : NODE_STANDBY);
  TRACE_EXIT();
}
