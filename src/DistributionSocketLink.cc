// DistributionSocketLink.cc
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

#define GNET_EXPERIMENTAL

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include "Configurator.hh"
#include "DistributionManager.hh"
#include "DistributionLink.hh"
#include "DistributionSocketLink.hh"
#include "DistributionLinkListener.hh"

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
  active_client(NULL),
  active(false),
  canonical_name(NULL),
  server_port(DEFAULT_PORT),
  server_socket(NULL),
  server_enabled(false),
  configurator(conf),
  dist_manager(NULL),
  username(NULL),
  password(NULL),
  reconnect_attempts(DEFAULT_ATTEMPTS),
  reconnect_interval(DEFAULT_INTERVAL),
  heartbeat_count(0)
{
}


//! Destructs the socket link.
DistributionSocketLink::~DistributionSocketLink()
{
  remove_client(NULL);
  
  if (server_socket != NULL)
    {
      gnet_tcp_socket_delete(server_socket);
    }
  if (canonical_name != NULL)
    {
      g_free(canonical_name);
    }
  if (username != NULL)
    {
      g_free(username);
    }
  if (password != NULL)
    {
      g_free(password);
    }
}


//! Initialize the link.
bool
DistributionSocketLink::init()
{
  TRACE_ENTER("DistributionSocketLink::init");

  // How am I?
  GInetAddr *ia = gnet_inetaddr_gethostaddr();
  g_assert(ia != NULL);
  canonical_name = gnet_inetaddr_get_canonical_name(ia);
  gnet_inetaddr_delete(ia);

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
  TRACE_ENTER("DistributionSocketLink::heartbeat");

  heartbeat_count++;
  
  time_t current_time = time(NULL);

  // See if we have some clients that need reconncting.
  list<Client *>::iterator i = clients.begin();
  while (i != clients.end())
    {
      Client *c = *i;
      if (c->reconnect_count > 0 &&
          c->reconnect_time != 0 && current_time >= c->reconnect_time &&
          c->canonical_name != NULL)
        {
          c->reconnect_count--;
          c->reconnect_time = 0;
          
          TRACE_MSG("Reconnecting to " << c->canonical_name << " " <<  c->server_port);
          gnet_tcp_socket_connect_async(c->canonical_name, c->server_port, static_async_connected, c);
        }
      i++;
    }

  // Periodically distribute state, in case the master crashes.
  if (heartbeat_count % 60 == 0 && active)
    { 
      send_state();
    }
                                         
  TRACE_EXIT();
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
      if (c->socket != NULL && c->iochannel != NULL)
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
  GURL *client_url = gnet_url_new(url.c_str());

  add_client(client_url->hostname, client_url->port);

  gnet_url_delete(client_url);
}


bool
DistributionSocketLink::disconnect_all()
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;

  active_client = NULL;

  while (i != clients.end())
    {
      Client *c = *i;
      
      if (c->iochannel != NULL)
        {
          g_io_channel_unref(c->iochannel);
          c->iochannel = NULL;
        }

      if (c->socket != NULL)
        {
          gnet_tcp_socket_delete(c->socket);
          c->socket = NULL;
        }

      if (c->watch != 0)
        {
          g_source_remove(c->watch);
        }
      c->watch = 0;
      c->watch_flags = 0;

      c->reconnect_count = 0;
      c->reconnect_time = 0;          

      ret = true;
      i++;
    }

  set_me_active();
  
  return ret;
}


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


//! Attempt to become the active node.
/*!
 *  \return true if the claim was successfull.
 */
bool
DistributionSocketLink::claim()
{
  TRACE_ENTER("DistributionSocketLink::claim");
  bool ret = true;

  if (active_client != NULL)
    {
      // Another client is active. Politely request to become
      // active client.
      send_claim(active_client);
      ret = false;
    }
  else if (!active && clients.size() > 0)
    {
      // No one is active. Just force is to be active
      // potential problem when more client do this simultaneously...
      send_new_master();
      active = true;
    }
  else
    {
      // No one active, no other clients. Be happy.
      active = true;
    }
  TRACE_EXIT();
  return ret;
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
          // FIXME: report to user.
          enabled = false;
        }
    }
  else if (server_enabled && !enabled)
    {
      // Switching from enabled to disabled.
      if (server_socket != NULL)
        {
          gnet_tcp_socket_delete(server_socket);
        }
      server_socket = NULL;
      disconnect_all();
      set_me_active();
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
  return false;
}


//! Returns whether the specified client exists.
/*!
 *  This method checks if the specified client is an existing remote
 *  client or the local client.
 */
bool
DistributionSocketLink::exists_client(gchar *host, gint port)
{
  bool ret = (port == server_port && strcmp(host, canonical_name) == 0);

  if (!ret)
    {
      Client *c = find_client_by_canonicalname(host, port);
      ret = (c != NULL);
    }

  return ret;
}


//! Adds a new client and connect to it.
bool
DistributionSocketLink::add_client(gchar *host, gint port)
{
  gchar *canonical_host = NULL;
  
  bool skip = exists_client(host, port);
  
  if (!skip)
    {
      // This client doesn't seem to exist. Now try the
      // canonical name of this client. 
      GInetAddr *ia =  gnet_inetaddr_new(host, port);
      if (ia != NULL)
        {
          canonical_host = gnet_inetaddr_get_canonical_name(ia);
          if (canonical_host != NULL)
            {
              skip = exists_client(canonical_host, port);

              // Use this canonical name instead of the supplied
              // host name.
              host = canonical_host;
            }
          gnet_inetaddr_delete(ia);
        }
    }
  
  if (!skip)
    {
      // Client does not yet exists as far as we can see.
      // So, create a new one.
      Client *client = new Client;
        
      client->packet.create();
      client->link = this;
      client->canonical_name = g_strdup(host);
      client->server_port = port;
      
      clients.push_back(client);
      gnet_tcp_socket_connect_async(host, port, static_async_connected, client);
    }

  g_free(canonical_host);
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
  bool ret = true;
  
  bool exists = exists_client(host, port);
  if (exists)
    {
      // Already have a client with this name/port
      Client *old_client = find_client_by_canonicalname(host, port);

      if (old_client == NULL)
        {
          // Iek this is me.
          ret =  false;
        }
      else if (old_client != client)
        {
          // It's a remote client, but not the same one.
          bool connected = (old_client->socket != NULL || old_client->iochannel != NULL);
          
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
      g_free(client->canonical_name);

      client->canonical_name = g_strdup(host);
      client->server_port = port;
    }
  
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
  
  if (client == active_client)
    {
      // Client to be removed is active. Unset active client. 
      active_client = NULL;
      ret = true;
    }

  while (i != clients.end())
    {
      if (client == NULL || *i == client)
        {
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


//! Finds a remote client by its canonical name and port.
DistributionSocketLink::Client *
DistributionSocketLink::find_client_by_canonicalname(gchar *name, gint port)
{
  Client *ret = NULL;
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if ((*i)->server_port == port && strcmp((*i)->canonical_name, name) == 0)
        {
          ret = *i;
        }
      i++;
    }
  return ret;
}


//! Sets the specified remote client as active.
void
DistributionSocketLink::set_active(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::set_active")
  active_client = client;
  active = false;

  if (dist_manager != NULL)
    {
      dist_manager->active_changed(false);
    }
  TRACE_EXIT();
}


//! Sets the local client as active.
void
DistributionSocketLink::set_me_active()
{
  TRACE_ENTER("DistributionSocketLink::set_me_active");
  active_client = NULL;
  active = true;

  if (dist_manager != NULL)
    {
      dist_manager->active_changed(true);
    }
  TRACE_EXIT();
}


//! Sets the specified client active.
void
DistributionSocketLink::set_active(gchar *cname, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_active", cname << " " << port);

  Client *c = find_client_by_canonicalname(cname, port);

  if (c != NULL)
    {
      // It's a remote client. mark it active.
      set_active(c);
    }
  else if (port == server_port && strcmp(cname, canonical_name) == 0)
    {
      // Its ME!
      set_me_active();
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

      if (c != client && c->iochannel != NULL)
        {
          TRACE_MSG("sending to " << c->canonical_name << ":" << c->server_port);
          
          guint bytes_written = 0;
          GIOError error = g_io_channel_write(c->iochannel, packet.get_buffer(), size, &bytes_written);
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

  if (client->iochannel != NULL)
    {
      gint size = packet.bytes_written();
  
      // Length.
      packet.poke_ushort(0, size);
      
      guint bytes_written = 0;
      GIOError error = g_io_channel_write(client->iochannel, packet.get_buffer(), size, &bytes_written);
    }
  
  TRACE_EXIT();
}


//! Processed an incoming packet.
void
DistributionSocketLink::process_client_packet(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::process_client_packet");
  PacketBuffer &packet = client->packet;
  
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
  packet.pack_string(canonical_name);
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
    
  TRACE_MSG("Hello from " << name << ":" << port << " " << user);

  if ( (username == NULL || (user != NULL && strcmp(username, user) == 0)) &&
       (password == NULL || (pass != NULL && strcmp(password, pass) == 0)))
    {
      bool ok = set_canonical(client, name, port);
  
      if (ok)
        {
          // Welcome!
          send_welcome(client);

          // The connecting client offers the active client.
          // This info will be received in the client list.
          // So, we no longer know who's active...
          set_active(NULL);

          // And send the list of client we are connected to.
          // WITHOUT info about who's active on out side.
          send_client_list(client);
        }
      else
        {
          // Duplicate client. inform client that it's bogus and close.
          TRACE_MSG("Removing duplicate");
          send_duplicate(client);
          remove_client(client);
        }
    }
  else
    {
      // Incorrect password.
      TRACE_MSG("Client access denied");
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
  PacketBuffer &packet = client->packet;
  
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
  packet.pack_string(canonical_name);
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

  TRACE_MSG("Welcome from " << name << ":" << port);

  // Change the canonical name in out client list.
  bool ok = set_canonical(client, name, port);

  if (ok)
    {
      // All, ok. Send list of known client.s
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

      if (active)
        {
          // The sender of the packet is active.
          flags |= CLIENTLIST_IAM_ACTIVE;
        }
      else if (active_client != NULL)
        {
          // Another client is active. The canonical name/port
          // of the client will be put in the client list.
          flags |= CLIENTLIST_HAS_ACTIVE_REF;
        }

      int count = 0;
      gint clients_pos = packet.bytes_written();
      
      packet.pack_ushort(0);		// number of clients in the list
      packet.pack_ushort(flags);	// flags.

      if (flags & CLIENTLIST_HAS_ACTIVE_REF)
        {
          // Put active client in the packet.
          packet.pack_string(active_client->canonical_name);
          packet.pack_ushort(active_client->server_port);
        }

      // Put known client in the list.
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;

          // Only put client in the list we are connected to,
          // but not the client to which we send the list.
          if (c != client && c->socket != NULL && c->iochannel != NULL)
            {
              count++;
              gint pos = packet.bytes_written();

              packet.pack_ushort(0);			// Length
              packet.pack_string(c->canonical_name);	// Canonical name
              packet.pack_ushort(c->server_port);	// Listen port.

              // Size of the client data.
              packet.poke_ushort(pos, packet.bytes_written() - pos);
            }
          
          i++;
        }

      // Put packet size in the packet and send.
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
  bool sender_active = flags & CLIENTLIST_IAM_ACTIVE;
  bool has_active_ref = flags & CLIENTLIST_HAS_ACTIVE_REF;

  if (sender_active)
    {
      // Mark the sender as active.
      set_active(client);
    }
  else if (has_active_ref)
    {
      // Find out how is active.
      gchar *cname = packet.unpack_string();
      gint port = packet.unpack_ushort();

      set_active(cname, port);

      g_free(cname);
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


//! Requests to become active.
void
DistributionSocketLink::send_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_claim");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_CLAIM);

  packet.pack_ushort(0);
  
  send_packet(client, packet);
  TRACE_EXIT();
}


//! Handles a request from a remote client to become active.
void
DistributionSocketLink::handle_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");
  PacketBuffer &packet = client->packet;
  
  gint count = packet.unpack_ushort();
  bool was_active = active;

  // Marks client as active and tell everyone.
  set_active(client);
  send_new_master();

  // If I was previously active, distribute state.
  if (was_active)
    {
      send_state();
    }
  
  TRACE_EXIT();
}


//! Informs the specified client (or all remote clients) that a new client is now active.
void
DistributionSocketLink::send_new_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_new_master");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_NEW_MASTER);

  gchar *name = NULL;
  gint port = 0;
  
  if (active_client == NULL)
    {
      // I've become active
      name = canonical_name;
      port = server_port;
    }
  else
    {
      // Another remote client becomes active
      name = active_client->canonical_name;
      port = active_client->server_port;
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

  PacketBuffer &packet = client->packet;

  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();
  gint count = packet.unpack_ushort();

  TRACE_MSG("new master from "
            << client->canonical_name << ":" << client->server_port << " -> "
            << name << ":" << port);

  set_active(name, port);
  
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
              state_map[id]->set_state(id, active, data, datalen);
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


/* ************************************************************ */


bool
DistributionSocketLink::start_async_server()
{
  TRACE_ENTER("DistributionSocketLink::start_async_server");
  bool ret = false;
  
  /* Create the server */
  server_socket = gnet_tcp_socket_server_new(server_port);
  if (server_socket != NULL)
    {
      /* Print the address */
      GInetAddr *addr = gnet_tcp_socket_get_inetaddr(server_socket);
      g_assert(addr);
      gchar *name = gnet_inetaddr_get_canonical_name(addr);
      g_assert (name);
      gint port = gnet_inetaddr_get_port(addr);
      TRACE_MSG("Async echoserver running on " << name << ":" << port);
      gnet_inetaddr_delete(addr);
      g_free(name);
      
      gnet_tcp_socket_server_accept_async(server_socket, static_async_accept, this);
      ret = true;
    }

  TRACE_RETURN(ret);
  return ret;
}


void
DistributionSocketLink::async_accept(GTcpSocket *server_socket, GTcpSocket *client_socket)
{
  (void) server_socket;
  
  TRACE_ENTER("DistributionSocketLink::async_accept");
  if (client_socket != NULL)
    {
      GInetAddr *addr = gnet_tcp_socket_get_inetaddr(client_socket);
      g_assert(addr);
      gchar *name = gnet_inetaddr_get_canonical_name(addr);
      g_assert(name);
      gint port = gnet_inetaddr_get_port(addr);

      TRACE_MSG("Accepted connection from " << name << ":" << port);
      gnet_inetaddr_delete(addr);

      Client *client =  new Client;
      client->packet.create();
              
      client->link = this;
      client->socket = client_socket;
      client->canonical_name = NULL;
      client->server_name = name;
      client->server_port = 0;
      client->iochannel = gnet_tcp_socket_get_iochannel(client_socket);
      client->watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
      client->reconnect_count = 0;
      client->reconnect_time = 0;
      
      g_assert(client->iochannel);
      
      client->watch = g_io_add_watch (client->iochannel, (GIOCondition) client->watch_flags,
                                      static_async_io, client);
      
      clients.push_back(client);
    }
  TRACE_EXIT();
}


bool
DistributionSocketLink::async_io(GIOChannel *iochannel, GIOCondition condition, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::async_io");
  bool ret = true;

  g_assert(client != NULL);
  g_assert(iochannel != NULL);
  
  // check for socket error
  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      TRACE_MSG("Client socket error " << client->canonical_name << ":" << client->server_port);
      ret = false;
    }

  // process input
  if (ret && (condition & G_IO_IN))
    {
      TRACE_MSG("Client ready for read " << client->canonical_name << ":" << client->server_port);

      guint bytes_read = 0;
      guint bytes_to_read = 4;

      if (client->packet.bytes_available() >= 4)
        {
          bytes_to_read = client->packet.peek_ushort(0) - 4;
        }
      
      GIOError error = g_io_channel_read(iochannel, client->packet.get_write_ptr(), bytes_to_read, &bytes_read);
      
      if (error != G_IO_ERROR_NONE)
	{
	  TRACE_MSG("Client read error " << client->canonical_name << ":" << client->server_port << " " <<  error);
	  ret = false;
	}
      else if (bytes_read == 0)
	{
	  TRACE_MSG("Connection closed from " << client->canonical_name << ":" << client->server_port);
	  ret = false;
	}
      else
	{
          TRACE_MSG("Read from " << client->canonical_name << ":" << client->server_port << " " <<  bytes_read);
	  g_assert(bytes_read > 0);

	  client->packet.write_ptr += bytes_read;

          if (client->packet.peek_ushort(0) == client->packet.bytes_written())
            {
              process_client_packet(client);
            }
	}
      
    }

  if (!ret)
    {
      // Socket error. disable client.
      if (client->iochannel != NULL)
        {
          g_io_channel_unref(client->iochannel);
          client->iochannel = NULL;
        }

      if (client->socket != NULL)
        {
          gnet_tcp_socket_delete(client->socket);
          client->socket = NULL;
        }

      if (client->watch != 0)
        {
          g_source_remove(client->watch);
        }
      
      client->watch = 0;
      client->watch_flags = 0;

      client->reconnect_count = reconnect_attempts;
      client->reconnect_time = time(NULL) + reconnect_interval;

      if (active_client == client)
        {
          set_active(NULL);
        }
    }
  
  TRACE_EXIT();
  return ret;
}


void 
DistributionSocketLink::async_connected(GTcpSocket *socket, GInetAddr *ia,
                                        GTcpSocketConnectAsyncStatus status,
                                        Client *client)
{
  TRACE_ENTER("DistributionSocketLink::async_connected");

  g_assert(client != NULL);
  
  if (status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK)
    {
      TRACE_MSG("Error: could not connect. status=" << status);

      gnet_tcp_socket_delete(socket);
      client->socket = NULL;
    }
  else
    {
      TRACE_MSG("connected to " << client->server_name << ":" << client->server_port);

      g_assert(ia != NULL);
      g_assert(socket != NULL);
      
      client->reconnect_count = 0;
      client->reconnect_time = 0;
      client->socket = socket;
      client->server_name = gnet_inetaddr_get_canonical_name(ia);
      client->server_port = gnet_inetaddr_get_port(ia);
      client->iochannel = gnet_tcp_socket_get_iochannel(socket);
      client->watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
      client->watch = g_io_add_watch(client->iochannel, (GIOCondition) client->watch_flags,
                                     static_async_io, client);
      
      gnet_inetaddr_delete(ia);
      send_hello(client);
    }
  TRACE_EXIT();
}


void
DistributionSocketLink::static_async_accept(GTcpSocket* server, GTcpSocket* client, gpointer data)
{
  DistributionSocketLink *link = (DistributionSocketLink *)data;
  g_assert(link != NULL);
  
  link->async_accept(server, client);
}


gboolean
DistributionSocketLink::static_async_io(GIOChannel *iochannel, GIOCondition condition,
                                        gpointer data)
{
  Client *client =  (Client *)data;
  
  g_assert(client != NULL);
  return client->link->async_io(iochannel, condition, client);
}


void 
DistributionSocketLink::static_async_connected(GTcpSocket *socket, GInetAddr *ia,
                                               GTcpSocketConnectAsyncStatus status, gpointer data)
{
  Client *client =  (Client *)data;

  g_assert(client != NULL);
  client->link->async_connected(socket, ia, status, client);
}


/***********************************************************************/

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
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_USERNAME, &user);
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
  is_set = configurator->get_value(CFG_KEY_DISTRIBUTION_TCP + CFG_KEY_DISTRIBUTION_TCP_PASSWORD, &passwd);
  if (!is_set)
    {
      password = NULL;
    }
  else
    {
      password = g_strdup(passwd.c_str());
    }
}


void
DistributionSocketLink::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("DistributionSocketLink:config_changed_notify", key);

  read_configuration();
  
  TRACE_EXIT();
}
