// DistributionSocketLink.cc
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
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include "nls.h"

#define GNET_EXPERIMENTAL
#include <gnet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "Configurator.hh"
#include "DistributionManager.hh"
#include "DistributionLink.hh"
#include "DistributionSocketLink.hh"
#include "DistributionLinkListener.hh"
#include "GNetSocketDriver.hh"

const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP = "distribution/tcp";
const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PORT = "/port";
const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_USERNAME = "/username";
const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_PASSWORD = "/password";
const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS = "/reconnect_attempts";
const string DistributionSocketLink::CFG_KEY_DISTRIBUTION_TCP_INTERVAL = "/reconnect_interval";


//! Construct a new socket link.
/*!
 *  \param conf Configurator to use.
 */
DistributionSocketLink::DistributionSocketLink(Configurator *conf) :
  dist_manager(NULL),
  configurator(conf),
  master_client(NULL),
  i_am_master(false),
  myname(NULL),
  server_port(DEFAULT_PORT),
  server_socket(NULL),
  username(NULL),
  password(NULL),
  server_enabled(false),
  reconnect_attempts(DEFAULT_ATTEMPTS),
  reconnect_interval(DEFAULT_INTERVAL),
  heartbeat_count(0)
{
  socket_driver = new GNetSocketDriver();
  socket_driver->set_listener(this);
}


//! Destructs the socket link.
DistributionSocketLink::~DistributionSocketLink()
{
  remove_client(NULL);
  
  if (server_socket != NULL)
    {
      delete server_socket;
    }
  if (myname != NULL)
    {
      g_free(myname);
    }
  if (username != NULL)
    {
      g_free(username);
    }
  if (password != NULL)
    {
      g_free(password);
    }
  if (socket_driver != NULL)
    {
      delete socket_driver;
    }
}


//! Initialize the link.
bool
DistributionSocketLink::init()
{
  TRACE_ENTER("DistributionSocketLink::init");

  // Who am I?
  myname = socket_driver->get_my_canonical_name();

  // I'm master.
  master_client = NULL;
  i_am_master = true;

  // Read all tcp link configuration.
  read_configuration();
  configurator->add_listener(CFG_KEY_DISTRIBUTION_TCP, this);
  
  TRACE_EXIT();
  return true;
}


//! Periodic heartbeat.
void
DistributionSocketLink::heartbeat()
{
  if (server_enabled)
    {
      heartbeat_count++;
  
      time_t current_time = time(NULL);

      // See if we have some clients that need reconncting.
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;
          if (c->reconnect_count > 0 &&
              c->reconnect_time != 0 && current_time >= c->reconnect_time &&
              c->hostname != NULL)
            {
              c->reconnect_count--;
              c->reconnect_time = 0;
          
              dist_manager->log(_("Reconnecting to %s:%d."), c->hostname, c->port);
              socket_driver->connect(c->hostname, c->port, c);
            }
          i++;
        }

      // Periodically distribute state, in case the master crashes.
      if (heartbeat_count % 60 == 0 && i_am_master)
        { 
          send_state();
        }
    }
}


//! Returns the total number of peer in the network.
int
DistributionSocketLink::get_number_of_peers()
{
  int count = 0;
  
  list<Client *>::iterator i = clients.begin();
  while (i != clients.end())
    {
      Client *c = *i;
      if (c->socket != NULL)
        {
          count++;
        }
      i++;
    }

  return count;
}


//! Join the WR network.
void
DistributionSocketLink::join(string url)
{
  if (server_enabled)
    {
      GURL *client_url = gnet_url_new(url.c_str());

      if (client_url != NULL)
        {
          add_client(client_url->hostname, client_url->port);
        }
      gnet_url_delete(client_url);
    }
}


//! Disconnects all clients.
bool
DistributionSocketLink::disconnect_all()
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;

  master_client = NULL;

  while (i != clients.end())
    {
      Client *c = *i;
      
      if (c->socket != NULL)
        {
          dist_manager->log(_("Disconnecting %s:%d."), c->hostname, c->port);
          delete c->socket;
          c->socket = NULL;
        }

      c->reconnect_count = 0;
      c->reconnect_time = 0;          

      ret = true;
      i++;
    }

  set_me_master();
  
  return ret;
}


//! Reconnects all clients.
bool
DistributionSocketLink::reconnect_all()
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;
  
  while (i != clients.end())
    {
      (*i)->reconnect_count = reconnect_attempts;
      (*i)->reconnect_time = time(NULL) - 1;          

      ret = true;
      i++;
    }

  return ret;
}


//! Attempt to become the master node.
/*!
 *  \return true if the claim was successfull.
 */
bool
DistributionSocketLink::claim()
{
  TRACE_ENTER("DistributionSocketLink::claim");
  bool ret = true;

  if (master_client != NULL)
    {
      // Another client is master. Politely request to become
      // master client.
      send_claim(master_client);
      ret = false;
    }
  else if (!i_am_master && clients.size() > 0)
    {
      // No one is master. Just force to be master
      // potential problem when more client do this simultaneously...
      send_new_master();
      i_am_master = true;
    }
  else
    {
      // No one master, no other clients. Be happy.
      i_am_master = true;
    }
  TRACE_EXIT();
  return ret;
}


//! Lock the master status. Claim will be denied when locked.
bool
DistributionSocketLink::set_lock_master(bool lock)
{
  master_locked = lock;
  return true;
}


//! Sets the username and password.
void
DistributionSocketLink::set_user(string user, string pw)
{
  g_free(username);
  g_free(password);

  username = g_strdup(user.c_str());
  password = g_strdup(pw.c_str());
}


//! Sets the distribution manager for callbacks.
void
DistributionSocketLink::set_distribution_manager(DistributionLinkListener *dll)
{
  dist_manager = dll;
}


//! Enable/disable distributed operation.
bool
DistributionSocketLink::set_enabled(bool enabled)
{
  bool ret = server_enabled;

  if (!server_enabled && enabled)
    {
      // Switching from disabled to enabled;
      if (!start_async_server())
        {
          // We did not succeed in starting the server. Arghh.
          dist_manager->log(_("Could not enable network operation."));
          enabled = false;
        }
    }
  else if (server_enabled && !enabled)
    {
      // Switching from enabled to disabled.
      if (server_socket != NULL)
        {
          dist_manager->log(_("Disabling network operation."));

          delete server_socket;
        }
      server_socket = NULL;
      disconnect_all();
      set_me_master();
    }

  server_enabled = enabled;
  return ret;
}


//! Register a distributed state.
bool
DistributionSocketLink::register_state(DistributedStateID id,
                                       DistributedStateInterface *dist_state)
{
  state_map[id] = dist_state;
  return true;
}


//! Unregister a distributed state.
bool
DistributionSocketLink::unregister_state(DistributedStateID id)
{
  (void) id;
  return false;
}


//! Force is state distribution.
bool
DistributionSocketLink::push_state(DistributedStateID id, unsigned char *buffer, int size)
{
  TRACE_ENTER("DistributionSocketLink::push_state");
  PacketBuffer packet;
  packet.create();
  init_packet(packet, PACKET_STATEINFO);

  gchar *name = NULL;
  gint port = 0;
  
  //bool have_master = get_master(&name, &port);
  packet.pack_string(name);
  packet.pack_ushort(port);
  g_free(name);
  
  packet.pack_ushort(1);
  packet.pack_ushort(size);
  packet.pack_ushort(id);
  packet.pack_raw(buffer, size);

  send_packet_broadcast(packet);
  return true;
  TRACE_EXIT();
}


//! Returns whether the specified client is this client.
bool
DistributionSocketLink::client_is_me(gchar *host, gint port)
{
  return
    (host != NULL) && (myname != NULL) &&
    (port == server_port) && (strcmp(host, myname) == 0);
}


//! Returns whether the specified client exists.
/*!
 *  This method checks if the specified client is an existing remote
 *  client or the local client.
 */
bool
DistributionSocketLink::exists_client(gchar *host, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::exists_client", host << " " << port);

  bool ret = (port == server_port && strcmp(host, myname) == 0);

  if (!ret)
    {
      Client *c = find_client_by_canonicalname(host, port);
      ret = (c != NULL);
    }

  TRACE_RETURN(ret);
  return ret;
}


//! Adds a new client and connect to it.
bool
DistributionSocketLink::add_client(gchar *host, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::add_client", host << " " << port);
  gchar *canonical_host = NULL;
  
  bool skip = exists_client(host, port);
  
  if (!skip)
    {
      // This client doesn't seem to exist. Now try the
      // canonical name of this client.
      canonical_host = socket_driver->canonicalize(host);
      if (canonical_host != NULL)
        {
          skip = exists_client(canonical_host, port);

          // Use this canonical name instead of the supplied
          // host name.
          host = canonical_host;
        }
    }
  
  if (!skip)
    {
      // Client does not yet exists as far as we can see.
      // So, create a new one.
      Client *client = new Client;
        
      client->packet.create();
      client->hostname = g_strdup(host);
      client->port = port;
      
      clients.push_back(client);
      dist_manager->log(_("Connecting to %s:%d."), host, port);
      socket_driver->connect(host, port, client);
    }

  g_free(canonical_host);
  return true;
  TRACE_EXIT();
}


//! Sets the canonical name of a client.
/*!
 *  This method also checks for duplicates.
 *
 *  \return true if the client is a duplicate. The canonical name will not
 *  changed if the client is a duplicate.
 */
bool
DistributionSocketLink::set_canonical(Client *client, gchar *host, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_canonical", host << " " << port); 
  bool ret = true;
  
  bool exists = exists_client(host, port);
  if (exists)
    {
      // Already have a client with this name/port
      Client *old_client = find_client_by_canonicalname(host, port);

      if (old_client == NULL || (port == server_port && strcmp(host, myname) == 0))
        {
          // Iek this is me.
          TRACE_MSG("It'me me");
          ret =  false;
        }
      else if (old_client != client)
        {
          // It's a remote client, but not the same one.
          bool connected = (old_client->socket != NULL);
          
          if (connected)
            {
              // Already connected to this client.
              // Duplicate.
              ret = false;
            }
          else
            {
              // Client exist, but is not connected.
              // Silently remove the old client.
              remove_client(old_client);
            }
        }
      else
        {
          // it's ok. It's same one.
        }
    }

  if (ret)
    {
      // No duplicate, so change the canonical name.
      g_free(client->hostname);

      client->hostname = g_strdup(host);
      client->port = port;
    }

  TRACE_RETURN(ret);
  return ret;
}


//! Removes a client (or all clients)
/*!
 *  Network connections to removed client are closed.
 *
 *  \param client client to remove, or NULL if all clients to be removed.
 *
 *  \return true if a client has been removed
 */
bool
DistributionSocketLink::remove_client(Client *client)
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;
  
  if (client == master_client)
    {
      // Client to be removed is master. Unset master client. 
      master_client = NULL;
      ret = true;
    }

  while (i != clients.end())
    {
      if (client == NULL || *i == client)
        {
          dist_manager->log(_("Removing client %s:%d."), (*i)->hostname, (*i)->port);
          delete *i;
          i = clients.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }

  return ret;
}


//! Check if a client point is stil valid....
bool
DistributionSocketLink::is_client_valid(Client *client)
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;
  
  while (!ret && i != clients.end())
    {
      if (*i == client)
        {
          ret = true;
        }
      i++;
    }

  return ret;
}


//! Finds a remote client by its canonical name and port.
DistributionSocketLink::Client *
DistributionSocketLink::find_client_by_canonicalname(gchar *name, gint port)
{
  Client *ret = NULL;
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if ((*i)->port == port && strcmp((*i)->hostname, name) == 0)
        {
          ret = *i;
        }
      i++;
    }
  return ret;
}


//! Returns the master client.
bool
DistributionSocketLink::get_master(gchar **name, gint *port) const
{
  bool ret = false;
  
  *name = NULL;
  *port = 0;
  
  if (i_am_master)
    {
      *name = g_strdup(myname);
      *port = server_port;
      ret = true;
    }
  else if (master_client != NULL)
    {
      *name = g_strdup(master_client->hostname);
      *port = master_client->port;
      ret = true;
    }
  return ret;
}


//! Sets the specified remote client as master.
void
DistributionSocketLink::set_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::set_master")
  master_client = client;
  i_am_master = false;

  if (dist_manager != NULL)
    {
      dist_manager->master_changed(false);
    }
  TRACE_EXIT();
}


//! Sets the local client as master.
void
DistributionSocketLink::set_me_master()
{
  TRACE_ENTER("DistributionSocketLink::set_me_master");
  master_client = NULL;
  i_am_master = true;

  if (dist_manager != NULL)
    {
      dist_manager->master_changed(true);
    }
  TRACE_EXIT();
}


//! Sets the specified client master.
void
DistributionSocketLink::set_master(gchar *hostname, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_master", hostname << " " << port);

  Client *c = find_client_by_canonicalname(hostname, port);

  if (c != NULL)
    {
      // It's a remote client. mark it master.
      dist_manager->log(_("Client %s:%d is now master."), c->hostname, c->port);
      set_master(c);
    }
  else if (port == server_port && strcmp(hostname, myname) == 0)
    {
      // Its ME!
      dist_manager->log(_("I'm now master."));
      set_me_master();
    }
  else
    {
      // Huh???
      TRACE_MSG("Iek");
    }
  
  TRACE_EXIT();
}


//! Initialize an outgoing packet.
void
DistributionSocketLink::init_packet(PacketBuffer &packet, PacketCommand cmd)
{
  // Length.
  packet.pack_ushort(0);
  // Version
  packet.pack_byte(1);
  // Flags
  packet.pack_byte(0);
  // Command
  packet.pack_ushort(cmd);
}


//! Sends the specified packet to all clients.
void
DistributionSocketLink::send_packet_broadcast(PacketBuffer &packet)
{
  TRACE_ENTER("DistributionSocketLink::send_packet_broadcast");
  
  send_packet_except(packet, NULL);
  
  TRACE_EXIT();
}


//! Sends the specified packet to all clients with the exception of one client.
void
DistributionSocketLink::send_packet_except(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_packet_except");
  
  gint size = packet.bytes_written();

  // Length.
  packet.poke_ushort(0, size);

  list<Client *>::iterator i = clients.begin();
  while (i != clients.end())
    {
      Client *c = *i;

      if (c != client && c->socket != NULL)
        {
          TRACE_MSG("sending to " << c->hostname << ":" << c->port);
          
          int bytes_written = 0;
          c->socket->write(packet.get_buffer(), size, bytes_written);
        }
      i++;
    }

  TRACE_EXIT();
}


//! Sends the specified packet to the specified client.
void
DistributionSocketLink::send_packet(Client *client, PacketBuffer &packet)
{
  TRACE_ENTER("DistributionSocketLink::send_packet");

  if (client->socket != NULL)
    {
      gint size = packet.bytes_written();
  
      // Length.
      packet.poke_ushort(0, size);
      
      int bytes_written = 0;
      client->socket->write(packet.get_buffer(), size, bytes_written);
    }
  
  TRACE_EXIT();
}


//! Processed an incoming packet.
void
DistributionSocketLink::process_client_packet(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::process_client_packet");
  PacketBuffer &packet = client->packet;

  client->claim_count = 0;
  
  gint size = packet.unpack_ushort();
  g_assert(size == packet.bytes_written());

  gint version = packet.unpack_byte();
  gint flags = packet.unpack_byte();

  TRACE_MSG("size = " << size << ", version = " << version << ", flags = " << flags);
  
  if (packet.bytes_available() >= size - 4)
    {
      gint type = packet.unpack_ushort();
      TRACE_MSG("type = " << type);

      switch (type)
        {
        case PACKET_HELLO:
          handle_hello(client);
          break;

        case PACKET_CLAIM:
          handle_claim(client);
          break;

        case PACKET_WELCOME:
          handle_welcome(client);
          break;

        case PACKET_CLIENT_LIST:
          handle_client_list(client);
          break;

        case PACKET_NEW_MASTER:
          handle_new_master(client);
          break;

        case PACKET_STATEINFO:
          handle_state(client);
          break;

        case PACKET_DUPLICATE:
          handle_duplicate(client);
          break;

        case PACKET_CLAIM_REJECT:
          handle_claim_reject(client);
          break;
        }
    }

  packet.clear();
  TRACE_EXIT();
}


//! Sends a hello to the specified client.
void
DistributionSocketLink::send_hello(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_hello");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_HELLO);

  packet.pack_string(username);
  packet.pack_string(password);
  packet.pack_string(myname);
  packet.pack_ushort(server_port);
  
  send_packet(client, packet);
  TRACE_EXIT();
}


//! Handles a Hello from the specified client.
void
DistributionSocketLink::handle_hello(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_hello");
  PacketBuffer &packet = client->packet;
  
  gchar *user = packet.unpack_string();
  gchar *pass = packet.unpack_string();
  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();

  dist_manager->log(_("Client %s:%d saying hello."), name, port);
  
  if ( (username == NULL || (user != NULL && strcmp(username, user) == 0)) &&
       (password == NULL || (pass != NULL && strcmp(password, pass) == 0)))
    {
      bool ok = set_canonical(client, name, port);
  
      if (ok)
        {
          // Welcome!
          send_welcome(client);
          
          // And send the list of client we are connected to.
          send_client_list(client);
        }
      else
        {
          // Duplicate client. inform client that it's bogus and close.
          dist_manager->log(_("Client %s:%d is duplicate."), name, port);

          send_duplicate(client);
          remove_client(client);
        }
    }
  else
    {
      // Incorrect password.
      dist_manager->log(_("Client %s:%d access denied."), name, port);
      remove_client(client);
    }
  
  g_free(name);
  g_free(user);
  g_free(pass);
    
  TRACE_EXIT();
}



//! Sends a duplicate to the specified client.
void
DistributionSocketLink::send_duplicate(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_duplicate");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_DUPLICATE);

  send_packet(client, packet);
  TRACE_EXIT();
}


//! Handles a duplicate for the specified client.
void
DistributionSocketLink::handle_duplicate(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_duplicate");
  dist_manager->log(_("Removing duplicate client %s:%d."), client->hostname, client->port);
  remove_client(client);

  TRACE_EXIT();
}


//! Sends a welcome message to the specified client
void
DistributionSocketLink::send_welcome(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_welcome");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_WELCOME);

  // My Info
  packet.pack_string(myname);
  packet.pack_ushort(server_port);

  send_packet(client, packet);
  TRACE_EXIT();
}


//! Handles a welcome message from the specified client.
void
DistributionSocketLink::handle_welcome(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_welcome");
  PacketBuffer &packet = client->packet;
  
  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();

  dist_manager->log(_("Client %s:%d is welcoming us."), name, port);
  
  // Change the canonical name in out client list.
  bool ok = set_canonical(client, name, port);

  if (ok)
    {
      // The connected client offers the master client.
      // This info will be received in the client list.
      // So, we no longer know who's master...
      set_master(NULL);
      
      // All, ok. Send list of known client.
      // WITHOUT info about who's master on out side.
      send_client_list(client);
    }
  else
    {
      // Duplicate.
      send_duplicate(client);
      remove_client(client);
    }
  
  TRACE_EXIT();
}


//! Sends the list of known clients to the specified client.
void
DistributionSocketLink::send_client_list(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_client_list");
  
  if (clients.size() > 0)
    {
      PacketBuffer packet;
      packet.create();
      init_packet(packet, PACKET_CLIENT_LIST);

      // The receiver must forward this to clients it knows.
      int flags = CLIENTLIST_FORWARDABLE;

      if (i_am_master)
        {
          TRACE_MSG("I'm master");
          // The sender of the packet is master.
          flags |= CLIENTLIST_IAM_ACTIVE;
        }
      else if (master_client != NULL)
        {
          TRACE_MSG("Someone else is master");
          // Another client is master. The canonical name/port
          // of the client will be put in the client list.
          flags |= CLIENTLIST_HAS_ACTIVE_REF;
        }

      int count = 0;
      gint clients_pos = packet.bytes_written();
      
      packet.pack_ushort(0);		// number of clients in the list
      packet.pack_ushort(flags);	// flags.

      if (flags & CLIENTLIST_HAS_ACTIVE_REF)
        {
          // Put master client in the packet.
          packet.pack_string(master_client->hostname);
          packet.pack_ushort(master_client->port);
        }

      // Put known client in the list.
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;

          // Only put client in the list we are connected to,
          // but not the client to which we send the list.
          if (c != client && c->socket != NULL)
            {
              count++;
              gint pos = packet.bytes_written();

              packet.pack_ushort(0);		// Length
              packet.pack_string(c->hostname);	// Canonical name
              packet.pack_ushort(c->port);	// Listen port.

              // Size of the client data.
              packet.poke_ushort(pos, packet.bytes_written() - pos);
            }
          
          i++;
        }

      // Put packet size in the packet and send.
      // dist_manager->log(_("Sending client-list to %s:%d."), client->hostname, client->port);
      packet.poke_ushort(clients_pos, count);
      send_packet(client, packet);
    }
  
  TRACE_EXIT();
}


//! Handles a client list from the specified client.
void
DistributionSocketLink::handle_client_list(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_client_list");
  
  PacketBuffer &packet = client->packet;

  // Extract data.
  gint num_clients = packet.unpack_ushort();
  gint pos = packet.bytes_read();
  gint flags = packet.unpack_ushort();

  bool forward = flags & CLIENTLIST_FORWARDABLE;
  bool sender_master = flags & CLIENTLIST_IAM_ACTIVE;
  bool has_master_ref = flags & CLIENTLIST_HAS_ACTIVE_REF;
  
  // dist_manager->log(_("Received client-list from %s:%d."), client->hostname, client->port);
  
  if (sender_master)
    {
      // Mark the sender as master.
      TRACE_MSG("Sender is master");
      set_master(client);
    }
  else if (has_master_ref)
    {
      // Find out how is master.
      gchar *hostname = packet.unpack_string();
      gint port = packet.unpack_ushort();

      set_master(hostname, port);
      TRACE_MSG(hostname << ":" << port << " is master");

      g_free(hostname);
    }

  // Forward if required.
  if (forward)
    {
      // Forward onlt once!
      flags &= ~CLIENTLIST_FORWARDABLE;

      // And forward.
      packet.poke_ushort(pos, flags);
      send_packet_except(packet, client);
    }

  // Loop over remote clients.
  for (int i = 0; i < num_clients; i++)
    {
      // Extract data.
      gint pos = packet.bytes_read();
      gint size = packet.unpack_ushort();
      gchar *name = packet.unpack_string();
      gint port = packet.unpack_ushort();

      if (name != NULL && port != 0 && !exists_client(name, port))
        {
          // A new one, connect to it.
          add_client(name, port);
        }

      // Skip trailing junk...
      size -= (packet.bytes_read() - pos);
      packet.skip(size);

      g_free(name);
    }

  TRACE_EXIT();
}


//! Requests to become master.
void
DistributionSocketLink::send_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_claim");

  if (client->next_claim_time == 0 || time(NULL) >= client->next_claim_time)
    {
      PacketBuffer packet;

      dist_manager->log(_("Requesting master status from %s:%d."), client->hostname, client->port);

      packet.create();
      init_packet(packet, PACKET_CLAIM);
      
      packet.pack_ushort(0);
      
      client->next_claim_time = time(NULL) + 10;

      send_packet(client, packet);

      if (client->claim_count >= 3)
        {
          dist_manager->log(_("Client timeout from %s:%d."), client->hostname, client->port);

          // Socket error. disable client.
          if (client->socket != NULL)
            {
              delete client->socket;
              client->socket = NULL;
            }

          client->reconnect_count = reconnect_attempts;
          client->reconnect_time = time(NULL) + reconnect_interval;

          if (master_client == client)
            {
              set_master(NULL);
            }
        }
      client->claim_count++;
    }
  
  TRACE_EXIT();
}


//! Handles a request from a remote client to become master.
void
DistributionSocketLink::handle_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");
  PacketBuffer &packet = client->packet;
  
  /*gint count = */ packet.unpack_ushort();
  
  if (i_am_master && master_locked)
    {
      dist_manager->log(_("Rejecting master request from client %s:%d."),
                        client->hostname, client->port);
      send_claim_reject(client);
    }
  else
    {
      dist_manager->log(_("Acknowledging master request from client %s:%d."),
                        client->hostname, client->port);
  
      bool was_master = i_am_master;

      // Marks client as master
      set_master(client);
      assert(!i_am_master);

      // If I was previously master, distribute state.
      if (was_master)
        {
          //dist_manager->log(_("Transferring state to client %s:%d."),
          //                  client->hostname, client->port);
          send_state();
        }
  
      // And tell everyone we have a new master.
      send_new_master();
    }
  
  TRACE_EXIT();
}



//! Inform that the claim has been rejected.
void
DistributionSocketLink::send_claim_reject(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_claim_reject");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_CLAIM_REJECT);

  send_packet(client, packet);
  TRACE_EXIT();
}


//! Handles a rejection of my claim.
void
DistributionSocketLink::handle_claim_reject(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");

  if (client != master_client)
    {
      dist_manager->log(_("Non-master client %s:%d rejected master request."),
                        client->hostname, client->port);
    }
  else
    {
      dist_manager->log(_("Client %s:%d rejected master request, delaying."),
                        client->hostname, client->port);
      client->reject_count++;
      int count = client->reject_count;

      if (count > 6)
        {
          count = 6;
        }
      
      client->next_claim_time = time(NULL) + 5 * count;
    }
  
  TRACE_EXIT();
}

//! Informs the specified client (or all remote clients) that a new client is now master.
void
DistributionSocketLink::send_new_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_new_master");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_NEW_MASTER);

  gchar *name = NULL;
  gint port = 0;
  
  if (master_client == NULL)
    {
      // I've become master
      name = myname;
      port = server_port;
    }
  else
    {
      // Another remote client becomes master
      name = master_client->hostname;
      port = master_client->port;
    }

  packet.pack_string(name);
  packet.pack_ushort(port);
  packet.pack_ushort(0);

  if (client != NULL)
    {
      send_packet(client, packet);
    }
  else
    {
      send_packet_broadcast(packet);
    }

  TRACE_EXIT();

}


//! Handles a new master event.
void
DistributionSocketLink::handle_new_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_new_master");

  for (list<Client *>::iterator i = clients.begin(); i != clients.end(); i++)
    {
      (*i)->reject_count = 0;
    }

  PacketBuffer &packet = client->packet;

  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();
  /* gint count = */ packet.unpack_ushort();

  dist_manager->log(_("Client %s:%d is now the new master."), name, port);
  
  TRACE_MSG("new master from "
            << client->hostname << ":" << client->port << " -> "
            << name << ":" << port);

  set_master(name, port);
  
  g_free(name);
  
  TRACE_EXIT();
}


// Distributes the current state.
void
DistributionSocketLink::send_state()
{
  TRACE_ENTER("DistributionSocketLink:send_state");

  PacketBuffer packet;
  packet.create();
  init_packet(packet, PACKET_STATEINFO);

  gchar *name = NULL;
  gint port = 0;
  
  // bool have_master = get_master(&name, &port);
  packet.pack_string(name);
  packet.pack_ushort(port);
  g_free(name);
  
  packet.pack_ushort(state_map.size());
  
  map<DistributedStateID, DistributedStateInterface *>::iterator i = state_map.begin();
  while (i != state_map.end())
    {
      DistributedStateID id = i->first;
      DistributedStateInterface *itf = i->second;

      guint8 *data = NULL;
      gint size = 0;

      if (itf->get_state(id, &data, &size))
        {
          packet.pack_ushort(size);
          packet.pack_ushort(id);
          packet.pack_raw(data, size);

          delete [] data;
        }
      else
        {
          packet.pack_ushort(0);
          packet.pack_ushort(id);
        }
      i++;
    }

  send_packet_broadcast(packet);
  TRACE_EXIT();
}


//! Handles new state from a remote client.
void
DistributionSocketLink::handle_state(Client *client)
{
  TRACE_ENTER("DistributionSocketLink:handle_state");
  PacketBuffer &packet = client->packet;

  bool will_i_become_master = false;

  // dist_manager->log(_("Reveived state from client %s:%d."), client->hostname, client->port);
  
  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();

  if (name != NULL)
    {
      will_i_become_master = client_is_me(name, port);
      g_free(name);
    }
  
  gint size = packet.unpack_ushort();
  
  for (int i = 0; i < size; i++)
    {
      gint datalen = packet.unpack_ushort();
      DistributedStateID id = (DistributedStateID) packet.unpack_ushort();

      if (datalen != 0)
        {
          guint8 *data = NULL;
          if (packet.unpack_raw(&data, datalen) != 0)
            {
              state_map[id]->set_state(id, will_i_become_master, data, datalen);
            }
          else
            {
              TRACE_MSG("Illegal state packet");
              break;
            }
              
        }
    }

  if (dist_manager != NULL)
    {
      // Inform distribution manager that all state is processed.
      dist_manager->state_transfer_complete();
    }
  
  TRACE_EXIT();
}


bool
DistributionSocketLink::start_async_server()
{
  TRACE_ENTER("DistributionSocketLink::start_async_server");
  bool ret = false;
  
  /* Create the server */
  server_socket = socket_driver->listen(server_port, NULL);
  if (server_socket != NULL)
    {
      dist_manager->log(_("Network operation started."));
      ret = true;
    }

  TRACE_RETURN(ret);
  return ret;
}


void
DistributionSocketLink::socket_accepted(SocketConnection *scon, SocketConnection *ccon)
{
  (void) scon;
  
  TRACE_ENTER("DistributionSocketLink::socket_accepted");
  if (ccon != NULL)
    {
      dist_manager->log(_("Accepted new client."));

      Client *client =  new Client;
      client->packet.create();
              
      client->socket = ccon;
      client->hostname = NULL;
      client->port = 0;
      client->reconnect_count = 0;
      client->reconnect_time = 0;

      ccon->set_data(client);
      clients.push_back(client);
    }
  TRACE_EXIT();
}


void
DistributionSocketLink::socket_io(SocketConnection *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_io");
  bool ret = true;

  Client *client = (Client *)data;
  
  g_assert(client != NULL);

  if (!is_client_valid(client))
    {
      TRACE_RETURN("Invalid client");
      return;
      
    }

  int bytes_read = 0;
  int bytes_to_read = 4;

  if (client->packet.bytes_available() >= 4)
    {
      bytes_to_read = client->packet.peek_ushort(0) - 4;
    }
      
  bool ok = con->read(client->packet.get_write_ptr(), bytes_to_read, bytes_read);
      
  if (!ok)
    {
      dist_manager->log(_("Client %s:%d read error, closing."), client->hostname, client->port);
      ret = false;
    }
  else if (bytes_read == 0)
    {
      dist_manager->log(_("Client %s:%d closed connection."), client->hostname, client->port);
      ret = false;
    }
  else
    {
      // TRACE_MSG("Read from " << client->hostname << ":" << client->port << " " <<  bytes_read);
      g_assert(bytes_read > 0);
      
      client->packet.write_ptr += bytes_read;
      
      if (client->packet.peek_ushort(0) == client->packet.bytes_written())
        {
          process_client_packet(client);
        }
    }
  

  if (!ret)
    {
      // Socket error. disable client.
      if (client->socket != NULL)
        {
          delete client->socket;
          client->socket = NULL;
        }

      client->reconnect_count = reconnect_attempts;
      client->reconnect_time = time(NULL) + reconnect_interval;

      if (master_client == client)
        {
          set_master(NULL);
        }
    }
  
  TRACE_EXIT();
  return;
}


void 
DistributionSocketLink::socket_connected(SocketConnection *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_connected");

  Client *client = (Client *)data;
  
  g_assert(client != NULL);
  g_assert(con != NULL);
 
  if (!is_client_valid(client))
    {
      TRACE_RETURN("Invalid client");
      return;
      
    }
 
  dist_manager->log(_("Client %s:%d connected."), client->hostname, client->port);
  
  client->reconnect_count = 0;
  client->reconnect_time = 0;
  client->socket = con;

  send_hello(client);

  TRACE_EXIT();
}


void
DistributionSocketLink::socket_closed(SocketConnection *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_closed");
  (void) con;
  
  Client *client = (Client *)data;
  assert(client != NULL);

  if (!is_client_valid(client))
    {
      TRACE_RETURN("Invalid client");
      return;
      
    }

  // Socket error. Disable client.
  if (client->socket != NULL)
    {
      dist_manager->log(_("Client %s:%d closed connection."), client->hostname, client->port);
      delete client->socket;
      client->socket = NULL;
    }
  else
    {
      dist_manager->log(_("Could not connect to client %s:%d."), client->hostname, client->port);
    }
  
  client->reconnect_count = reconnect_attempts;
  client->reconnect_time = time(NULL) + reconnect_interval;
  
  if (master_client == client)
    {
      set_master(NULL);
    }
  TRACE_EXIT();
}


//! Read the configuration from the configurator.
void
DistributionSocketLink::read_configuration()
{
  bool is_set;

  int old_port = server_port;
  
  const char *port = getenv("WORKRAVE_PORT");
  if (port != NULL)
    {
      server_port = atoi(port);
    }
  else
    {
  
      // TCP listen port
      is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_PORT, &server_port);
      if (!is_set)
        {
          server_port = DEFAULT_PORT;
        }
    }

  if (old_port != server_port && server_enabled)
    {
      set_enabled(false);
      set_enabled(true);
    }
  
  // Reconnect interval
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_INTERVAL,
                                   &reconnect_interval);
  if (!is_set)
    {
      reconnect_interval = DEFAULT_INTERVAL;
    }

  // TCP listen port
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS,
                                   &reconnect_attempts);
  if (!is_set)
    {
      reconnect_attempts = DEFAULT_ATTEMPTS;
    }

  // Username
  string user;
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_USERNAME,
                                   &user);
  if (!is_set)
    {
      username = NULL;
    }
  else
    {
      username = g_strdup(user.c_str());
    }

  // Password
  string passwd;
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_PASSWORD,
                                   &passwd);
  if (!is_set)
    {
      password = NULL;
    }
  else
    {
      password = g_strdup(passwd.c_str());
    }
}


//! Notification from the configurator that the configuration has changed.
void
DistributionSocketLink::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("DistributionSocketLink:config_changed_notify", key);
 
  dist_manager->log(_("Configuration modified. Reloading."));
  read_configuration();
  
  TRACE_EXIT();
}
