// DistributionManager.cc
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "DistributionManager.hh"
#include "DistributionSocketLink.hh"
#include "DistributionLogListener.hh"
#include "DistributionListener.hh"
#include "Configurator.hh"

#ifdef WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif

const string DistributionManager::CFG_KEY_DISTRIBUTION = "distribution";
const string DistributionManager::CFG_KEY_DISTRIBUTION_ENABLED = "/enabled";
const string DistributionManager::CFG_KEY_DISTRIBUTION_PEERS = "/peers";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP = "distribution/tcp";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP_PORT = "/port";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP_USERNAME = "/username";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP_PASSWORD = "/password";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS = "/reconnect_attempts";
const string DistributionManager::CFG_KEY_DISTRIBUTION_TCP_INTERVAL = "/reconnect_interval";


//! Constructs a new DistributionManager.
DistributionManager::DistributionManager() :
  distribution_enabled(false),
  link(NULL),
  state(NODE_ACTIVE)
{
}


//! Destructs this DistributionManager. 
DistributionManager::~DistributionManager()
{
  delete link;
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


//! Returns true if this node is master.
bool
DistributionManager::is_master() const
{
  return state == NODE_ACTIVE;
}


//! Returns the name of the current master (or "" if requesting node is master)
string
DistributionManager::get_master_id() const
{
  return state == NODE_ACTIVE ? get_my_id() : current_master;
}


//! Returns the id of this node.
string
DistributionManager::get_my_id() const
{
  string id;
  
  if (link != NULL)
    {
      id = link->get_id();
    }

  return id;
}


//! Requests to become master.
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



//! Locks the current master status.
/*!
 *  If this node has locked its master status, all requests from remote hosts
 *  to become master will be denied.
 */
bool
DistributionManager::set_lock_master(bool lock)
{
  bool ret = false;
  
  if (link != NULL)
    {
      ret = link->set_lock_master(lock);
    }
  
  return ret;
}


//! Connect to the specified URL.
bool
DistributionManager::connect(string url)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->connect(url);
      ret = true;
    }
  return ret;
}


//! Disconnects from client with the specified id.
bool
DistributionManager::disconnect(string id)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->disconnect(id);
      ret = true;
    }
  return ret;
}


//! Disconnects from all remote hosts.
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


//! Reconnects to all remote hosts.
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
DistributionManager::register_client_message(DistributionClientMessageID id,
                                             DistributionClientMessageType type,
                                             DistributionClientMessageInterface *callback)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->register_client_message(id, type, callback);
      ret = true;
    }
  return ret;
}


//! Unregister the specified state callback.
bool
DistributionManager::unregister_client_message(DistributionClientMessageID id)
{
  bool ret = false;
  
  if (link != NULL)
    {
      link->unregister_client_message(id);
      ret = true;
    }
  return ret;
}


//! Register a control listener.
bool
DistributionManager::add_listener(DistributionListener *listener)
{
  bool ret = true;

  ListenerIter i = listeners.begin();
  while (ret && i != listeners.end())
    {
      DistributionListener *l = *i;

      if (listener == l)
        {
          // Already added. Skip
          ret = false;
        }
      
      i++;
    }
  
  if (ret)
    {
      // not found -> add
      listeners.push_back(listener);
    }

  return ret;
}


//! Unregister a control listener.
bool
DistributionManager::remove_listener(DistributionListener *listener)
{
  bool ret = false;

  ListenerIter i = listeners.begin();
  while (!ret && i != listeners.end())
    {
      DistributionListener *l = *i;

      if (listener == l)
        {
          // Found. Remove
          i = listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }

  return ret;
}


//! Broadcasts a client message to all
bool
DistributionManager::broadcast_client_message(DistributionClientMessageID id, PacketBuffer &buffer)
{
  bool ret = false;
  
  if (link != NULL)
    {
      ret = link->broadcast_client_message(id, buffer);
    }
  return ret;
}


//! Event from Link that our 'master' status changed.
void
DistributionManager::master_changed(bool new_master, char *id)
{
  TRACE_ENTER("DistributionManager::master_changed");
  state = (new_master ? NODE_ACTIVE : NODE_STANDBY);

  if (id != NULL)
    {
      // id 
      current_master = id;
    }
  else
    {
      current_master = "";
    }
  
  TRACE_EXIT();
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
      connect(peer);
      ret = true;
    }

  write_peers();

  TRACE_EXIT();
  return ret;
}


//! Removes the specified peer.
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
  return ret;
}


void
DistributionManager::set_peers(string peers, bool connect)
{
  TRACE_ENTER_MSG("DistributionManager::set_peers", peers);
  parse_peers(peers, connect);
  write_peers();
  TRACE_EXIT();
}


//
void
DistributionManager::parse_peers(string peers, bool doconnect)
{
  TRACE_ENTER_MSG("DistributionManager::parse_peers", peers);
  peer_urls.clear();

  std::string::size_type pos = peers.find(',');
  while (pos != std::string::npos)
    {
      string peer = peers.substr(0, pos);
      peers = peers.substr(pos + 1);

      if (peer != "")
        {
          sanitize_peer(peer);
          peer_urls.push_back(peer);

          if (doconnect)
            {
              connect(peer);
            }
        }
      
      pos = peers.find(',');
    }

  if (peers.length() > 0)
    {
      sanitize_peer(peers);
      peer_urls.push_back(peers);
      if (doconnect)
        {
          connect(peers);
        }
    }
  TRACE_EXIT();
}


//! Read all disbution manager configuration.
void
DistributionManager::read_configuration()
{
  bool is_set;

  // Distributed operation enabled or not.
  distribution_enabled = get_enabled();

  // Enable/Disable link.
  assert(link != NULL);
  link->set_enabled(distribution_enabled);

  // Peers
  const char *env = getenv("WORKRAVE_URL");
  if (env != NULL)
    {
      parse_peers(env);
    }
  else
    {
      string peer ;
      is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_PEERS, &peer);
      if (is_set)
        {
          parse_peers(peer);
        }
    }
}


void
DistributionManager::write_peers()
{
  TRACE_ENTER("DistributionManager::write_peers");
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
  TRACE_EXIT();
}


//! Notification that the specified configuration key has changed.
void
DistributionManager::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("DistributionManager:config_changed_notify", key);

  read_configuration();
  
  TRACE_EXIT();
}


//! Notification that the specified configuration key has changed.
void
DistributionManager::log(char *fmt, ...)
{
  va_list va;

  va_start(va, fmt);
  
  time_t current_time = time (NULL);
  struct tm *lt = localtime(&current_time);

  char str[256];
  snprintf(str, 255, "[%02d/%02d/%02d %02d:%02d:%02d] ",
          lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900,
          lt->tm_hour, lt->tm_min, lt->tm_sec);

  char *ptr = str + strlen(str);
  vsnprintf(ptr, 255 - strlen(str), fmt, va);

  ptr = str + strlen(str);
  ptr[0] = '\n';
  ptr[1] = 0;

  log_messages.push_back(str);

  if (log_messages.size() > 500)
    {
      log_messages.erase(log_messages.begin());
    }
  fire_log_event(str);
}



//! Adds the log listener.
/*!
 *  \param listener listener to add.
 *
 *  \retval true listener successfully added.
 *  \retval false listener already added.
 */
bool
DistributionManager::add_log_listener(DistributionLogListener *listener)
{
  bool ret = true;

  LogListenerIter i = log_listeners.begin();
  while (ret && i != log_listeners.end())
    {
      DistributionLogListener *l = *i;

      if (listener == l)
        {
          // Already added. Skip
          ret = false;
        }
      
      i++;
    }
  
  if (ret)
    {
      // not found -> add
      log_listeners.push_back(listener);
    }

  return ret;
}


//! Removes the log listener.
/*!
 *  \param listener listener to stop monitoring.
 *
 *  \retval true listener successfully removed.
 *  \retval false listener not found.
 */
bool
DistributionManager::remove_log_listener(DistributionLogListener *listener)
{
  bool ret = false;

  LogListenerIter i = log_listeners.begin();
  while (!ret && i != log_listeners.end())
    {
      DistributionLogListener *l = *i;

      if (listener == l)
        {
          // Found. Remove
          i = log_listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }

  return ret;
}



//! Fire a log event.
void
DistributionManager::fire_log_event(string message)
{
  TRACE_ENTER_MSG("DistributionManager::fire_log_event", message);
  
  LogListenerIter i = log_listeners.begin();
  while (i != log_listeners.end())
    {
      DistributionLogListener *l = *i;
      if (l != NULL)
        {
          l->distribution_log(message);
        }
      i++;
    }
  
  TRACE_EXIT();
}


void
DistributionManager::fire_signon_client(char *id)
{
  TRACE_ENTER_MSG("DistributionManager::fire_signon_client", id);
  
  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      DistributionListener *l = *i;
      if (l != NULL)
        {
          l->signon_remote_client(id);
        }
      i++;
    }
  
  TRACE_EXIT();
}

void
DistributionManager::fire_signoff_client(char *id)
{
  TRACE_ENTER_MSG("DistributionManager::fire_signoff_client", id);
  
  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      DistributionListener *l = *i;
      if (l != NULL)
        {
          l->signoff_remote_client(id);
        }
      i++;
    }
  
  TRACE_EXIT();
}


void
DistributionManager::signon_remote_client(char *client_id)
{
  fire_signon_client(client_id);
}


void
DistributionManager::signoff_remote_client(char *client_id)
{
  fire_signoff_client(client_id);
}


//! Returns log messages.
list<string>
DistributionManager::get_logs() const
{
  return log_messages;
}


//! Returns all peers.
list<string>
DistributionManager::get_peers() const
{
  return peer_urls;
}


bool
DistributionManager::get_enabled() const
{
  bool ret = true;
  bool is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_ENABLED,
                                        &ret);
  if (!is_set)
    {
      ret = false;
    }

  return ret;
}


void
DistributionManager::set_enabled(bool b)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_ENABLED, b);
}


string
DistributionManager::get_username() const
{
  string ret;
  configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_USERNAME,
                          &ret);
  return ret;
}


void
DistributionManager::set_username(string name)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_USERNAME, name);
}


string
DistributionManager::get_password() const
{
  string ret;
  configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_PASSWORD,
                          &ret);
  return ret;
}


void
DistributionManager::set_password(string name)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_PASSWORD, name);
}


int
DistributionManager::get_port() const
{
  int ret;
  bool is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_PORT,
                                        &ret);
  if (!is_set)
    {
      ret = DEFAULT_PORT;
    }

  return ret;
}


void
DistributionManager::set_port(int v)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_PORT, v);
}


int
DistributionManager::get_reconnect_attempts() const
{
  int ret;
  bool is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS,
                                        &ret);
  if (!is_set)
    {
      ret = DEFAULT_ATTEMPTS;
    }

  return ret;
}


void
DistributionManager::set_reconnect_attempts(int v)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS, v);
}


int
DistributionManager::get_reconnect_interval() const
{
  int ret;
  bool is_set = configurator->get_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_INTERVAL,
                                        &ret);
  if (!is_set)
    {
      ret = DEFAULT_INTERVAL;
    }

  return ret;
}


void
DistributionManager::set_reconnect_interval(int v)
{
  configurator->set_value(CFG_KEY_DISTRIBUTION + CFG_KEY_DISTRIBUTION_TCP_INTERVAL, v);
}
