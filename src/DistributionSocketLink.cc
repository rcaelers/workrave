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

#include "DistributionLink.hh"
#include "DistributionSocketLink.hh"
#include "DistributionLinkListener.hh"

DistributionSocketLink::DistributionSocketLink() :
  active_client(NULL),
  active(false),
  canonical_name(NULL),
  server_port(0),
  server_socket(NULL),
  dist_manager(NULL)
{
  username = g_strdup("Pietje");
  password = g_strdup("Puk");
}


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


int
DistributionSocketLink::get_number_of_peers()
{
  return clients.size();
}


void
DistributionSocketLink::join(string url)
{
  GURL *client_url = gnet_url_new(url.c_str());

  add_client(client_url->hostname, client_url->port);

  gnet_url_delete(client_url);
}


bool
DistributionSocketLink::claim()
{
  TRACE_ENTER("DistributionSocketLink::claim");
  bool ret = true;

  if (active_client != NULL)
    {
      send_claim(active_client);
      ret = false;
    }
  else if (!active && clients.size() > 0)
    {
      send_new_master();
      active = true;
    }
  else
    {
      active = true;
    }
  TRACE_EXIT();
  return ret;
}


void
DistributionSocketLink::set_user(string user, string pw)
{
  if (username != NULL)
    {
      g_free(username);
    }
  if (password != NULL)
    {
      g_free(password);
    }

  username = g_strdup(user.c_str());
  password = g_strdup(pw.c_str());
}


void
DistributionSocketLink::set_distribution_manager(DistributionLinkListener *dll)
{
  dist_manager = dll;
}


bool
DistributionSocketLink::init(gint port)
{
  TRACE_ENTER("DistributionSocketLink::init");
  server_port = port;

  GInetAddr *ia = gnet_inetaddr_gethostaddr();
  canonical_name = gnet_inetaddr_get_canonical_name(ia);
  gnet_inetaddr_delete(ia);

  TRACE_MSG("My canonical name is " << canonical_name);
  bool ret = start_async_server();

  TRACE_EXIT();
  return ret;
}

bool
DistributionSocketLink::register_state(DistributedStateID id,
                                       DistributedStateInterface *dist_state)
{
  state_map[id] = dist_state;
  return true;
}


bool
DistributionSocketLink::unregister_state(DistributedStateID id)
{
  return false;
}


bool
DistributionSocketLink::add_client(gchar *host, gint port)
{
  Client *c = find_client_by_canonicalname(host, port);

  if (c == NULL && (port != server_port || strcmp(host, canonical_name) != 0))
    {
      Client *client = new Client;
        
      client->packet.create();
      client->link = this;
      client->socket = NULL;
      client->server_name = NULL;
      client->canonical_name = g_strdup(host);
      client->server_port = port;
      client->iochannel = NULL;
      client->watch_flags = 0;
      client->watch = 0;
        
      clients.push_back(client);
      gnet_tcp_socket_connect_async(host, port, static_async_connected, client);
    }
}


bool
DistributionSocketLink::remove_client(Client *client)
{
  list<Client *>::iterator i = clients.begin();
  bool ret = false;
  
  if (client == active_client)
    {
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
}

DistributionSocketLink::Client *
DistributionSocketLink::find_client_by_servername(gchar *name, gint port)
{
  Client *ret = NULL;
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if ((*i)->server_port == port && strcmp((*i)->server_name, name) == 0)
        {
          ret = *i;
        }
      i++;
    }
  return ret;
}


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



void
DistributionSocketLink::set_active(gchar *cname, gint port)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_active", cname << " " << port);
  Client *c = find_client_by_canonicalname(cname, port);

  if (c != NULL)
    {
      set_active(c);
    }
  else if (port == server_port && strcmp(cname, canonical_name) == 0)
    {
      set_me_active();
    }
  else
    {
      TRACE_MSG("Iek");
    }
  
  TRACE_EXIT();
}


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


void
DistributionSocketLink::send_packet_broadcast(PacketBuffer &packet)
{
  TRACE_ENTER("DistributionSocketLink::send_packet_broadcast");
  
  send_packet_except(packet, NULL);
  
  TRACE_EXIT();
}


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
        }
    }

  packet.clear();
  TRACE_EXIT();
}


void
DistributionSocketLink::send_hello(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_hello");
  
  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_HELLO);

  packet.pack_string(username);
  packet.pack_string(canonical_name);
  packet.pack_ushort(server_port);
  
  send_packet(client, packet);
  TRACE_EXIT();
}


void
DistributionSocketLink::handle_hello(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_hello");
  PacketBuffer &packet = client->packet;
  
  gchar *uname = packet.unpack_string();
  gchar *cname = packet.unpack_string();
  gint port = packet.unpack_ushort();
    
  TRACE_MSG("Hello from " << cname << ":" << port << " " << uname);

  if (client->canonical_name != NULL)
    {
      g_free(client->canonical_name);
    }
             
  client->canonical_name = cname;
  client->server_port = port;

  g_free(uname);
  
  send_welcome(client);

  // FIXME: fake not active...
  set_active(NULL);
  
  send_client_list(client);
  TRACE_EXIT();
}


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


void
DistributionSocketLink::handle_welcome(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_welcome");
  PacketBuffer &packet = client->packet;
  
  gchar *name = packet.unpack_string();
  gint port = packet.unpack_ushort();

  TRACE_MSG("Welcome from " << name << ":" << port);

  if (client->canonical_name != NULL)
    {
      g_free(client->canonical_name);
    }

  client->server_port = port;
  client->canonical_name = name;

  send_client_list(client);
  TRACE_EXIT();
}


void
DistributionSocketLink::send_client_list(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_client_list");
  
  if (clients.size() > 0)
    {
      PacketBuffer packet;
      packet.create();
      init_packet(packet, PACKET_CLIENT_LIST);

      int flags = CLIENTLIST_FORWARDABLE;

      if (active)
        {
          TRACE_MSG("Sender is active");
          flags |= CLIENTLIST_IAM_ACTIVE;
        }
      else if (active_client != NULL)
        {
          TRACE_MSG("Sender knows active");
          flags |= CLIENTLIST_HAS_ACTIVE_REF;
        }
      
      packet.pack_ushort(clients.size() - 1);
      packet.pack_ushort(flags);

      if (flags & CLIENTLIST_HAS_ACTIVE_REF)
        {
          packet.pack_string(active_client->canonical_name);
          packet.pack_ushort(active_client->server_port);
        }
      
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;

          if (c != client)
            {
              gint pos = packet.bytes_written();

              packet.pack_ushort(0);
              packet.pack_string(c->canonical_name);
              packet.pack_string(c->server_name);
              packet.pack_ushort(c->server_port);

              gint size = packet.bytes_written() - pos;

              packet.poke_ushort(pos, size);
            }
          i++;
        }
      send_packet(client, packet);
    }
  
  TRACE_EXIT();
}


void
DistributionSocketLink::handle_client_list(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_client_list");
  
  PacketBuffer &packet = client->packet;

  gint num_clients = packet.unpack_ushort();
  gint pos = packet.bytes_read();
  gint flags = packet.unpack_ushort();

  bool forward = flags & CLIENTLIST_FORWARDABLE;
  bool sender_active = flags & CLIENTLIST_IAM_ACTIVE;
  bool has_active_ref = flags & CLIENTLIST_HAS_ACTIVE_REF;

  if (sender_active)
    {
      set_active(client);
      TRACE_MSG("Sender is active");
    }
  else if (has_active_ref)
    {
      gchar *cname = packet.unpack_string();
      gint port = packet.unpack_ushort();

      TRACE_MSG("Sender knows active " << cname << ":" << port);
      set_active(cname, port);

      g_free(cname);
    }

  if (forward)
    {
      flags &= ~CLIENTLIST_FORWARDABLE;
        
      packet.poke_ushort(pos, flags);
      send_packet_except(packet, client);
      TRACE_MSG("Forwarded");
    }
      
  for (int i = 0; i < num_clients; i++)
    {
      gint pos = packet.bytes_read();
      gint size = packet.unpack_ushort();
      
      gchar *cname = packet.unpack_string();
      gchar *sname = packet.unpack_string();
      gint port = packet.unpack_ushort();
       
      TRACE_MSG("new client: " << cname << ":" << port);
      if (port != 0 && find_client_by_canonicalname(cname, port) == NULL)
        {
          TRACE_MSG("adding");
          add_client(cname, port);
        }

      size -= (packet.bytes_read() - pos);
      packet.skip(size);

      g_free(cname);
      g_free(sname);
    }

  TRACE_EXIT();
}


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


void
DistributionSocketLink::handle_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");
  PacketBuffer &packet = client->packet;
  
  gint count = packet.unpack_ushort();

  TRACE_MSG("Claim from " << client->canonical_name << ":" << client->server_port);

  bool was_active = active;
  
  set_active(client);
  send_new_master();

  if (was_active)
    {
      send_state();
    }
  TRACE_EXIT();
}


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
      name = canonical_name;
      port = server_port;
    }
  else
    {
      name = active_client->canonical_name;
      port = active_client->server_port;
    }

  TRACE_MSG("Sending new active " << name << ":" << port);
  
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


void
DistributionSocketLink::handle_new_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_new_master");

  PacketBuffer &packet = client->packet;

  gchar *cname = packet.unpack_string();
  gint port = packet.unpack_ushort();
  gint count = packet.unpack_ushort();

  TRACE_MSG("new master from " << client->canonical_name << ":" << client->server_port <<
            " :" << cname << ":" << port);

  set_active(cname, port);
  
  g_free(cname);
  
  TRACE_EXIT();
}


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

      bool ok = itf->get_state(id, &data, &size);

      TRACE_MSG("state " << size << " " << data);
      if (ok)
        {
          packet.pack_ushort(size);
          packet.pack_ushort(id);
          packet.pack(data, size);
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

      DistributedStateInterface *itf = state_map[id];

      if (datalen != 0)
        {
          guint8 *data = NULL;
          datalen = packet.unpack(&data);

          itf->set_state(id, data, datalen);
        }
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

      Client *client = new Client;

      client->packet.create();
      client->link = this;
      client->socket = client_socket;
      client->canonical_name = NULL;
      client->server_name = name;
      client->server_port = 0;
      client->iochannel = gnet_tcp_socket_get_iochannel(client_socket);
      client->watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

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
  
  /* check for socket error */
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
      remove_client(client);
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

      remove_client(client);
    }
  else
    {
      TRACE_MSG("connected to " << client->server_name << ":" << client->server_port);

      g_assert(ia != NULL);
      g_assert(socket != NULL);
      
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

