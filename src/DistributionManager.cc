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
#include <algorithm>
#include <assert.h>

#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"
#include "Configurator.hh"

DistributionManager *DistributionManager::instance = NULL;

const string DistributionManager::CFG_KEY_DISTRIBUTION = "distribution";
const string DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED = "/enabled";
const string DistributionManager::CFG_KEY_DISTRIBUTION_PEERS = "/peers";


//! Constructs a new DistributionManager.
DistributionManager::DistributionManager() :
  distribution_enabled(false),
  state_complete(true),
  link(NULL),
  state(NODE_STANDBY)
{
}


//! Destructs this DistributionManager. 
DistributionManager::~DistributionManager()
{
}


//! Initialize the DistributionManager from the specified Configurator.
void
DistributionManager::init(Configurator *conf)
{
  configurator = conf;

  // Create link to the outside world.
  DistributionSocketLink *socketlink = new DistributionSocketLink(conf);
  socketlink->set_distribution_manager(this);
  socketlink->init();
  link = socketlink;

  // Read configuration.
  read_configuration();
  configurator->add_listener(CFG_KEY_DISTRIBUTION, this);

}



//! Periodic heartbeat.
void
DistributionManager::heartbeart()
{
  // Forward heartbeat to link.
  if (link != NULL)
    {
      link->heartbeat();
    }
}


//! Returns the current distribution state of this node.
DistributionManager::NodeState
DistributionManager::get_state() const
{
  return state;
}


//! Returns the number of peers in the network. This node excluded.
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


//! Returns true if this node is active.
bool
DistributionManager::is_active() const
{
  return state == NODE_ACTIVE; // FIXME: && state_complete;
}


//! Requests to become active.
/*!
 *  \return true is the claim succeeded.
 */
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


//! Connect to the specified URL.
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
DistributionManager::disconnect_all()
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->disconnect_all();
      ret = true;
    }
  return ret;
}


bool
DistributionManager::reconnect_all()
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->reconnect_all();
      ret = true;
    }
  return ret;
}


//! Register the specified state callback.
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


//! Unregister the specified state callback.
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
  

//! Event from Link that our 'active' status changed.
void
DistributionManager::active_changed(bool new_active)
{
  TRACE_ENTER("DistributionManager::active_changed");
  state = (new_active ? NODE_ACTIVE : NODE_STANDBY);

  if (new_active)
    {
      state_complete = false;
    }
  TRACE_EXIT();
}


void
DistributionManager::state_transfer_complete()
{
  state_complete = true;
}


//! Cleanup the peer's name?.
void
DistributionManager::sanitize_peer(string &peer)
{
  int len = peer.length();
  while (len > 1 && (peer[0] == ' '))
    {
      peer = peer.substr(1, len - 1);
      len--;
    }
  
  while (len > 1 && (peer[len - 1] == ' '))
    {
      peer = peer.substr(0, len - 1);
      len--;
    }

  std::string::size_type pos = peer.find("://");
  
  if (pos == std::string::npos)
    {
      peer = "tcp://" + peer;
    }
}


//! Adds the specified peer.
bool
DistributionManager::add_peer(string peer)
{
  TRACE_ENTER_MSG("DistributionManager:add_peer", peer);
  bool ret = false;
  
  sanitize_peer(peer);

  list<string>::iterator i = find(peer_urls.begin(), peer_urls.end(), peer);

  if (i == peer_urls.end())
    {
      peer_urls.push_back(peer);
      join(peer);
      ret = true;
    }

  write_peers();

  TRACE_EXIT();
  return ret;
}


//! Removed the specified peer.
bool
DistributionManager::remove_peer(string peer)
{
  TRACE_ENTER_MSG("DistributionManager:remove_peer", peer);
  bool ret = false;
  
  sanitize_peer(peer);
  list<string>::iterator i = find(peer_urls.begin(), peer_urls.end(), peer);

  if (i != peer_urls.end())
    {
      peer_urls.erase(i);
      ret = true;
    }

  write_peers();
  
  TRACE_EXIT();
}


//
void
DistributionManager::set_peers(string peers)
{
  TRACE_ENTER_MSG("DistributionManager::set_peers", peers);
  peer_urls.clear();

  std::string::size_type pos = peers.find(',');
  while (pos != std::string::npos)
    {
      string peer = peers.substr(0, pos);
      peers = peers.substr(pos + 1);

      sanitize_peer(peer);
      peer_urls.push_back(peer);
      join(peer);

      pos = peers.find(',');
    }

  if (peers.length() > 0)
    {
      sanitize_peer(peers);
      peer_urls.push_back(peers);
      join(peers);
    }

  write_peers();
  
  TRACE_EXIT();
}


//! Read all disbution manager configuration.
void
DistributionManager::read_configuration()
{
  bool is_set;

  // Distributed operation enabled or not.
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_ENABLED, &distribution_enabled);
  if (!is_set)
    {
      distribution_enabled = false;
      configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_ENABLED, distribution_enabled);      
    }

  // Enable/Disable link.
  assert(link != NULL);
  link->set_enabled(distribution_enabled);

  // Peers
  const char *env = getenv("WORKRAVE_URL");
  if (env != NULL)
    {
      set_peers(env);
    }
  else
    {
      string peer;
      is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_PEERS, &peer);
      if (is_set)
        {
          set_peers(peer);
        }
    }
}


void
DistributionManager::write_peers()
{
  string peers;

  for (list<string>::iterator i = peer_urls.begin(); i != peer_urls.end(); i++)
    {
      if (i != peer_urls.begin())
        {
          peers += ",";
        }
      peers += (*i);
    }
  
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_PEERS, peers);
}


//! Notification that the specified configuration key has changed.
void
DistributionManager::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("DistributionManager:config_changed_notify", key);

  read_configuration();
  
  TRACE_EXIT();
}
