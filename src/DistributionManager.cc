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
#include "Configurator.hh"

DistributionManager *DistributionManager::instance = NULL;

const string DistributionManager::CFG_KEY_DISTRIBUTION = "distribution/";
const string DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED = "enabled";
const string DistributionManager::CFG_KEY_DISTRIBUTION_PEERS = "peers";

#define DEFAULT_PORT (4224)


DistributionManager::DistributionManager() :
  distribution_enabled(false),
  link(NULL),
  state(NODE_STANDBY)
{
}


DistributionManager::~DistributionManager()
{
}


void
DistributionManager::init(Configurator *conf)
{
  configurator = conf;

  DistributionSocketLink *socketlink = new DistributionSocketLink(conf);
  socketlink->set_distribution_manager(this);
  link = socketlink;

  read_configuration();
  configurator->add_listener(CFG_KEY_DISTRIBUTION, this);
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
DistributionManager::register_state(DistributedStateID id, DistributedStateInterface *dist_state)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->register_state(id, dist_state);
      ret = true;
    }
  return ret;
}


bool
DistributionManager::unregister_state(DistributedStateID id)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->unregister_state(id);
      ret = true;
    }
  return ret;
}
  

void
DistributionManager::active_changed(bool result)
{
  TRACE_ENTER("DistributionManager::active_changed");
  state = (result ? NODE_ACTIVE : NODE_STANDBY);
  TRACE_EXIT();
}


void
DistributionManager::read_configuration()
{
  bool is_set;

  // Distributed operation enabled or not.
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_ENABLED, &distribution_enabled);
  if (!is_set)
    {
      distribution_enabled = false;
    }

  // FIXME: temp
  const char *env = getenv("WORKRAVE_URL");
  if (env != NULL)
    {
      join(env);
    }
  else
    {
      string peer;
      is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_PEERS, &peer);
      if (is_set)
        {
          join(peer);
        }
    }
}


void
DistributionManager::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("DistributionManager:config_changed_notify", key);

  read_configuration();
  
  TRACE_EXIT();
}
