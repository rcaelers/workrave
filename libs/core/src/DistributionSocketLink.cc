// DistributionSocketLink.cc
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef PLATFORM_OS_MACOS
#  include "MacOSHelpers.hh"
#endif

#ifdef HAVE_DISTRIBUTION

#  include "debug.hh"
#  include <cassert>

#  include <iostream>
#  include <fstream>
#  include <algorithm>
#  include <filesystem>

#  include "nls.h"

#  include <cstdio>
#  include <cstdlib>
#  include <cstring>

#  include <csignal>

#  include "config/IConfigurator.hh"
#  include "core/CoreConfig.hh"
#  include "utils/AssetPath.hh"

#  include "DistributionManager.hh"
#  include "DistributionLink.hh"
#  include "DistributionSocketLink.hh"

#  ifdef PLATFORM_OS_WINDOWS
#    include "win32/ghmac.h"
#  endif

using namespace std;
using namespace workrave::utils;

//! Construct a new socket link.
/*!
 *  \param conf Configurator to use.
 */
DistributionSocketLink::DistributionSocketLink(workrave::config::IConfigurator::Ptr conf)
  : configurator(conf)
{
  socket_driver = SocketDriver::create();
  init_my_id();
}

//! Destructs the socket link.
DistributionSocketLink::~DistributionSocketLink()
{
  remove_client(nullptr);

  g_free(username);
  g_free(password);
  delete server_socket;
  delete socket_driver;
}

//! Initialize the link.
void
DistributionSocketLink::init()
{
  TRACE_ENTER("DistributionSocketLink::init");

  // I'm master.
  master_client = nullptr;
  i_am_master = true;

  // Read all tcp link configuration.
  read_configuration();
  configurator->add_listener(CoreConfig::CFG_KEY_DISTRIBUTION_TCP, this);
  configurator->add_listener(CoreConfig::CFG_KEY_DISTRIBUTION, this);

  TRACE_EXIT();
}

//! Periodic heartbeat.
void
DistributionSocketLink::heartbeat()
{
  if (server_enabled)
    {
      TRACE_ENTER("DistributionSocketLink::heartbeat");
      heartbeat_count++;

      time_t current_time = time(nullptr);

      // See if we have some clients that need reconncting.
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;
          if (c->type == CLIENTTYPE_DIRECT && c->reconnect_count > 0 && c->reconnect_time != 0 && current_time >= c->reconnect_time
              && c->hostname != nullptr)
            {
              c->reconnect_count--;
              c->reconnect_time = 0;

              dist_manager->log(_("Reconnecting to %s."), c->id == nullptr ? "Unknown" : c->id);

              if (c->socket != nullptr)
                {
                  c->socket->close();
                  delete c->socket;
                }

              ISocket *socket = socket_driver->create_socket();
              socket->set_data(c);
              socket->set_listener(this);
              socket->connect(c->hostname, c->port);

              c->socket = socket;
            }
          i++;
        }

      // Periodically distribute state, in case the master crashes.
      if (heartbeat_count % 30 == 0 && i_am_master)
        {
          send_client_message(DCMT_MASTER);
        }
      TRACE_EXIT();
    }
}

//! Initializes the network wrapper.
void
DistributionSocketLink::init_my_id()
{
  TRACE_ENTER("DistributionSocketLink::init_my_id");
  bool ok = false;
  string idfilename = AssetPath::get_home_directory() + "id";

  std::filesystem::path f(idfilename);
  if (std::filesystem::is_regular_file(f))
    {
      ifstream file(idfilename.c_str());

      if (file)
        {
          string id_str;
          file >> id_str;

          if (id_str.length() == WRID::STR_LENGTH)
            {
              ok = my_id.set(id_str);
            }
          file.close();
        }
    }

  if (!ok)
    {
      ofstream file(idfilename.c_str());

      file << my_id.str() << endl;
      file.close();
    }

  TRACE_EXIT();
}

//! Returns the id of this node.
string
DistributionSocketLink::get_my_id() const
{
  return my_id.str();
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
      if (c->socket != nullptr)
        {
          count++;
        }
      i++;
    }

  return count;
}

//! Join the WR network.
void
DistributionSocketLink::connect(string url)
{
  TRACE_ENTER_MSG("DistributionSocketLink::connect", url);
  if (network_enabled)
    {
      std::string::size_type pos = url.find("://");
      std::string hostport;

      if (pos == std::string::npos)
        {
          hostport = url;
        }
      else
        {
          hostport = url.substr(pos + 3);
        }

      pos = hostport.rfind(":");
      std::string host;
      std::string port = "0";

      if (pos == std::string::npos)
        {
          host = hostport;
        }
      else
        {
          host = hostport.substr(0, pos);
          port = hostport.substr(pos + 1);
        }

      add_client(nullptr, (gchar *)host.c_str(), atoi(port.c_str()), CLIENTTYPE_DIRECT);
    }
  TRACE_EXIT();
}

//! Disconnects the specified client.
void
DistributionSocketLink::disconnect(string id)
{
  TRACE_ENTER_MSG("DistributionSocketLink::disconnect", id);
  Client *c = find_client_by_id((gchar *)id.c_str());
  if (c != nullptr)
    {
      TRACE_MSG("close");
      close_client(c);
    }
  TRACE_EXIT();
}

//! Disconnects all clients.
bool
DistributionSocketLink::disconnect_all()
{
  TRACE_ENTER("DistributionSocketLink::disconnect_all");
  list<Client *>::iterator i = clients.begin();
  bool ret = false;

  master_client = nullptr;

  while (i != clients.end())
    {
      close_client(*i);
      ret = true;
      i++;
    }

  set_me_master();

  TRACE_EXIT();
  return ret;
}

//! Reconnects all clients.
bool
DistributionSocketLink::reconnect_all()
{
  TRACE_ENTER("DistributionSocketLink::reconnect_all");
  list<Client *>::iterator i = clients.begin();
  bool ret = false;

  while (i != clients.end())
    {
      if ((*i)->type == CLIENTTYPE_DIRECT)
        {
          close_client(*i, true);
          ret = true;
        }
      i++;
    }

  TRACE_EXIT();
  return ret;
}

//! Attempt to become the master node.
/*!
 *  \return true if the claim was successful.
 */
bool
DistributionSocketLink::claim()
{
  TRACE_ENTER("DistributionSocketLink::claim");
  bool ret = true;

  if (master_client != nullptr)
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
  TRACE_ENTER_MSG("DistributionSocketLink::set_user", user << " " << pw);
  g_free(username);
  g_free(password);

  username = g_strdup(user.c_str());
  password = g_strdup(pw.c_str());
  TRACE_EXIT();
}

//! Sets the distribution manager for callbacks.
void
DistributionSocketLink::set_distribution_manager(DistributionManager *dll)
{
  dist_manager = dll;
}

//! Enable/disable connecting to distributed operation.
bool
DistributionSocketLink::set_network_enabled(bool enabled)
{
  if (network_enabled && !enabled)
    {
      network_enabled = enabled;
      set_server_enabled(false);
      set_me_master();
    }
  else if (!network_enabled && enabled)
    {
      network_enabled = enabled;

      set_server_enabled(server_enabled);
    }

  return network_enabled;
}

//! Enable/disable listening for distributed operation.
bool
DistributionSocketLink::set_server_enabled(bool enabled)
{
  TRACE_ENTER_MSG("DistributionSocketLink:set_server_enabled", enabled);
  bool ret = server_enabled;

  if (!network_enabled)
    {
      // Don't start the server if the network is not enabled
      return ret;
    }

  if (server_socket == nullptr && enabled)
    {
      // Switching from disabled to enabled;
      if (!start_async_server())
        {
          // We did not succeed in starting the server. Arghh.
          dist_manager->log(_("Could not enable network operation."));
          enabled = false;
        }
    }
  else if (server_socket != nullptr && !enabled)
    {
      // Switching from enabled to disabled.
      if (server_socket != nullptr)
        {
          dist_manager->log(_("Disabling network operation."));

          delete server_socket;
        }
      server_socket = nullptr;
      disconnect_all();
    }

  server_enabled = enabled;
  TRACE_EXIT();
  return ret;
}

//! Register a distributed client message callback.
bool
DistributionSocketLink::register_client_message(DistributionClientMessageID id,
                                                DistributionClientMessageType type,
                                                IDistributionClientMessage *callback)
{
  ClientMessageListener sl;
  sl.listener = callback;
  sl.type = type;

  client_message_map[id] = sl;

  return true;
}

//! Unregister a distributed clien _message callback.
bool
DistributionSocketLink::unregister_client_message(DistributionClientMessageID id)
{
  (void)id;
  return false;
}

//! Force is state distribution.
bool
DistributionSocketLink::broadcast_client_message(DistributionClientMessageID dsid, PacketBuffer &buffer)
{
  TRACE_ENTER("DistributionSocketLink::broadcast_client_message");

  PacketBuffer packet;
  packet.create();
  init_packet(packet, PACKET_CLIENTMSG);

  string id = get_master();
  packet.pack_string(id);

  packet.pack_ushort(1);
  packet.pack_ushort(dsid);
  packet.pack_ushort(buffer.bytes_written());
  packet.pack_raw((unsigned char *)buffer.get_buffer(), buffer.bytes_written());

  send_packet_broadcast(packet);
  TRACE_EXIT();
  return true;
}

//! Returns whether the specified client is this client.
bool
DistributionSocketLink::client_is_me(gchar *id)
{
  return id != nullptr && strcmp(id, my_id.str().c_str()) == 0;
}

//! Returns whether the specified client exists.
/*!
 *  This method checks if the specified client is an existing remote
 *  client or the local client.
 */
bool
DistributionSocketLink::exists_client(gchar *id)
{
  TRACE_ENTER_MSG("DistributionSocketLink::exists_client", id);

  bool ret = client_is_me(id);

  if (!ret)
    {
      Client *c = find_client_by_id(id);
      ret = (c != nullptr);
    }

  TRACE_EXIT();
  return ret;
}

//! Adds a new client and connect to it.
bool
DistributionSocketLink::add_client(gchar *id, gchar *host, gint port, ClientType type, Client *peer)
{
  TRACE_ENTER_MSG("DistributionSocketLink::add_client",
                  (id != nullptr ? id : "NULL") << " " << (host != nullptr ? host : "NULL") << " " << port);

  gchar *canonical_host = nullptr;

  Client *c = find_client_by_canonicalname(host, port);
  if (c != nullptr && c->type == CLIENTTYPE_SIGNEDOFF && type == CLIENTTYPE_DIRECT)
    {
      if (c->id != nullptr)
        {
          dist_manager->signon_remote_client(c->id);
        }

      c->type = type;
      dist_manager->log(_("Connecting to %s."), host);

      if (c->socket != nullptr)
        {
          c->socket->close();
          delete c->socket;
        }

      ISocket *socket = socket_driver->create_socket();
      socket->set_data(c);
      socket->set_listener(this);
      socket->connect(host, port);

      c->socket = socket;
    }
  else
    {
      // Client does not yet exists as far as we can see.
      // So, create a new one.
      Client *client = new Client;

      client->type = type;
      client->peer = peer;
      client->packet.create();
      client->hostname = g_strdup(host);
      client->id = g_strdup(id);
      client->port = port;

      clients.push_back(client);

      if (client->id != nullptr)
        {
          dist_manager->signon_remote_client(client->id);
        }

      if (type == CLIENTTYPE_DIRECT)
        {
          dist_manager->log(_("Connecting to %s."), host);

          if (client->socket != nullptr)
            {
              client->socket->close();
              delete client->socket;
            }

          ISocket *socket = socket_driver->create_socket();
          socket->set_data(client);
          socket->set_listener(this);
          socket->connect(host, port);

          client->socket = socket;
        }
    }
  g_free(canonical_host);
  TRACE_EXIT();
  return true;
}

//! Sets the id of a client.
/*!
 *  This method also checks for duplicates.
 *
 *  \return true if the client is a duplicate. The id will not
 *  changed if the client is a duplicate.
 */
bool
DistributionSocketLink::set_client_id(Client *client, gchar *id)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_id", id);
  bool ret = true;

  bool exists = exists_client(id);
  if (exists)
    {
      // Already have a client with this name/port
      Client *old_client = find_client_by_id(id);

      if (old_client == nullptr)
        {
          // Iek this is me.
          TRACE_MSG("It'me me");
          ret = false;
        }
      else if (old_client != client)
        {
          TRACE_MSG("It's not me " << old_client->type << " " << old_client->socket);
          // It's a remote client, but not the same one.

          bool reuse =
            ((old_client->type == CLIENTTYPE_DIRECT || old_client->type == CLIENTTYPE_SIGNEDOFF) && old_client->socket == nullptr);

          TRACE_MSG("reuse " << reuse);
          if (reuse)
            {
              // Client exist, but is not connected.
              // Silently remove the old client.
              remove_client(old_client);
            }
          else
            {
              // Already connected to this client.
              // Duplicate.
              ret = false;
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
      g_free(client->id);
      g_free(client->hostname);
      client->id = g_strdup(id);
      client->hostname = nullptr;
      client->port = 0;

      if (client->id != nullptr)
        {
          dist_manager->signon_remote_client(client->id);
        }
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
 */
void
DistributionSocketLink::remove_client(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::remove_client");
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if (client == nullptr || *i == client || (*i)->peer == client)
        {
          if ((*i)->id != nullptr)
            {
              dist_manager->signoff_remote_client((*i)->id);
            }

          dist_manager->log(_("Removing client %s."), (*i)->id == nullptr ? "Unknown" : (*i)->id);
          delete *i;
          i = clients.erase(i);
        }
      else
        {
          i++;
        }
    }

  if (client == master_client)
    {
      // Client to be removed is master. Unset master client.
      master_client = nullptr;
    }
  TRACE_EXIT();
}

//! Removes all peers of the specified client.
void
DistributionSocketLink::remove_peer_clients(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::remove_peer_clients");

  list<Client *>::iterator i = clients.begin();
  while (i != clients.end())
    {
      if ((*i)->peer == client)
        {
          TRACE_MSG("Client " << (*i)->peer->id << " is peer of " << client->id);

          dist_manager->log(_("Removing client %s."), (*i)->id == nullptr ? "Unknown" : (*i)->id);
          send_signoff(nullptr, *i);

          if ((*i)->id != nullptr)
            {
              dist_manager->signoff_remote_client((*i)->id);
            }

          if (client == master_client)
            {
              // Connection to master is lost. Unset master client.
              set_master(nullptr);
            }

          delete *i;
          i = clients.erase(i);
        }
      else
        {
          i++;
        }
    }
  TRACE_EXIT();
}

//! Closes the connection to a client.
void
DistributionSocketLink::close_client(Client *client, bool reconnect /* = false*/)
{
  TRACE_ENTER_MSG("DistributionSocketLink::close_client", (client->id != nullptr ? client->id : "Unknown") << " " << reconnect);

  if (client->id != nullptr)
    {
      dist_manager->signoff_remote_client(client->id);
    }

  if (client == master_client)
    {
      // Client to be closed is master. Unset master client.
      set_master(nullptr);
    }

  if (client->type == CLIENTTYPE_DIRECT)
    {
      TRACE_MSG("Is direct");
      // Closing direct connection.
      dist_manager->log(_("Disconnecting %s"), client->id != nullptr ? client->id : "Unknown");

      // Inform the client that we are disconnecting.
      send_signoff(client, nullptr);

      if (client->socket != nullptr)
        {
          TRACE_MSG("Still connected");
          // Still connected. Disconnect.
          delete client->socket;
          client->socket = nullptr;

          if (reconnect)
            {
              TRACE_MSG("must reconnected");
              client->reconnect_count = reconnect_attempts;
              client->reconnect_time = time(nullptr) + 5;
            }
          else
            {
              TRACE_MSG("set signed off");
              client->reconnect_count = 0;
              client->reconnect_time = 0;
              client->type = CLIENTTYPE_SIGNEDOFF;
            }
        }

      send_signoff(nullptr, client);
      remove_peer_clients(client);
    }
  else if (client->type == CLIENTTYPE_SIGNEDOFF)
    {
      TRACE_MSG("is signed off");
      if (client->socket != nullptr)
        {
          TRACE_MSG("still connected");

          // Still connected. Disconnect.
          delete client->socket;
          client->socket = nullptr;

          client->reconnect_count = 0;
          client->reconnect_time = 0;
        }

      remove_peer_clients(client);
    }
  TRACE_EXIT();
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
  Client *ret = nullptr;
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if ((*i)->port == port && (*i)->hostname != nullptr && strcmp((*i)->hostname, name) == 0)
        {
          ret = *i;
        }
      i++;
    }
  return ret;
}

//! Finds a remote client by its id.
DistributionSocketLink::Client *
DistributionSocketLink::find_client_by_id(gchar *id)
{
  Client *ret = nullptr;
  list<Client *>::iterator i = clients.begin();

  while (i != clients.end())
    {
      if ((*i)->id != nullptr && strcmp((*i)->id, id) == 0)
        {
          ret = *i;
        }
      i++;
    }
  return ret;
}

//! Returns the master client.
string
DistributionSocketLink::get_master() const
{
  string id;

  if (i_am_master)
    {
      id = get_my_id();
    }
  else if (master_client != nullptr && master_client->id != nullptr)
    {
      id = master_client->id;
    }
  return id;
}

//! Sets the specified remote client as master.
void
DistributionSocketLink::set_master(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::set_master");
  master_client = client;
  i_am_master = false;

  if (dist_manager != nullptr)
    {
      dist_manager->master_changed(false, client != nullptr ? client->id : "");
    }
  TRACE_EXIT();
}

//! Sets the local client as master.
void
DistributionSocketLink::set_me_master()
{
  TRACE_ENTER("DistributionSocketLink::set_me_master");
  master_client = nullptr;
  i_am_master = true;

  if (dist_manager != nullptr)
    {
      dist_manager->master_changed(true, get_my_id());
    }
  TRACE_EXIT();
}

//! Sets the specified client master.
void
DistributionSocketLink::set_master_by_id(gchar *id)
{
  TRACE_ENTER_MSG("DistributionSocketLink::set_master", id);

  Client *c = find_client_by_id(id);

  if (c != nullptr)
    {
      // It's a remote client. mark it master.
      dist_manager->log(_("Client %s is now master."), c->id == nullptr ? "Unknown" : c->id);
      set_master(c);
    }
  else if (strcmp(id, get_my_id().c_str()) == 0)
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
  packet.pack_byte(3);
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

  send_packet_except(packet, nullptr);

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

      if (c != client && c->socket != nullptr)
        {
          int bytes_written = 0;
          try
            {
              c->socket->write(packet.get_buffer(), size, bytes_written);
            }
          catch (SocketException &)
            {
              TRACE_MSG("Failed to send");
            }
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

  if (client != nullptr && client->type == CLIENTTYPE_ROUTED)
    {
      TRACE_MSG("Must route packet.");

      if (client->id == nullptr)
        {
          TRACE_MSG("Client's ID == NULL");
        }

      packet.restart_read();
      int flags = packet.peek_byte(3);

      if (!(flags & PACKETFLAG_DEST) && client->id != nullptr)
        {
          assert(!(flags & PACKETFLAG_SOURCE));

          TRACE_MSG("Add destination " << client->id);

          packet.poke_byte(3, flags | PACKETFLAG_DEST);

          packet.insert(4, strlen(client->id) + 2);
          packet.poke_string(6, client->id);
        }
      client = client->peer;
    }

  if (client != nullptr && client->socket != nullptr)
    {
      if (client->id != nullptr)
        {
          TRACE_MSG("Sending to " << client->id);
        }

      gint size = packet.bytes_written();

      // Length.
      packet.poke_ushort(0, size);

      int bytes_written = 0;
      try
        {
          client->socket->write(packet.get_buffer(), size, bytes_written);
        }
      catch (SocketException &)
        {
          TRACE_MSG("Failed to send");
        }
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

  gint type = packet.unpack_ushort();
  TRACE_MSG("type = " << type);
  if (client != nullptr && client->id != nullptr)
    {
      TRACE_MSG("From = " << client->id);
    }

  Client *source = client;
  bool forward = true;
  if (flags & PACKETFLAG_SOURCE)
    {
      gchar *id = packet.unpack_string();

      if (!client_is_me(id))
        {
          TRACE_MSG("routed, source = " << id);

          source = find_client_by_id(id);

          if (source == nullptr)
            {
              TRACE_MSG("Unknown source. Dropping");
            }
          else if (source != client && source->peer != client)
            {
              TRACE_MSG("Illegal source in routing.");
              source = nullptr;
            }
        }
      else
        {
          TRACE_MSG("Cycle detected.");
          type = 0;
          flags = 0;
          source = nullptr;

          // Duplicate client. inform client that it's bogus and close.
          //           dist_manager->log(_("Client %s:%d is duplicate."), client->hostname, client->port);

          //           send_duplicate(client);
          //           remove_client(client);
        }
      g_free(id);
    }

  if (flags & PACKETFLAG_DEST)
    {
      gchar *id = packet.unpack_string();

      if (id != nullptr && !client_is_me(id))
        {
          TRACE_MSG("Destination = " << id);
          Client *dest = find_client_by_id(id);

          if (dest != nullptr)
            {
              TRACE_MSG("Forwarding to destination");
              forward_packet(packet, dest, source);
            }

          source = nullptr;
        }
      g_free(id);
    }

  TRACE_MSG("size = " << size << ", version = " << version << ", flags = " << flags);

  if (source != nullptr || type == PACKET_CLIENT_LIST)
    {
      switch (type)
        {
        case PACKET_HELLO1:
          handle_hello1(packet, source);
          forward = false;
          break;

        case PACKET_HELLO2:
          handle_hello2(packet, source);
          forward = false;
          break;

        case PACKET_SIGNOFF:
          handle_signoff(packet, source);
          break;

        case PACKET_CLAIM:
          handle_claim(packet, source);
          break;

        case PACKET_WELCOME:
          handle_welcome(packet, source);
          forward = false;
          break;

        case PACKET_CLIENT_LIST:
          forward = handle_client_list(packet, source, client);
          break;

        case PACKET_NEW_MASTER:
          handle_new_master(packet, source);
          break;

        case PACKET_CLIENTMSG:
          handle_client_message(packet, source);
          break;

        case PACKET_DUPLICATE:
          handle_duplicate(packet, source);
          forward = false;
          break;

        case PACKET_CLAIM_REJECT:
          handle_claim_reject(packet, source);
          break;
        }

      if (forward && find(clients.begin(), clients.end(), client) != clients.end())
        {
          forward_packet_except(packet, client, source);
        }
    }

  if (find(clients.begin(), clients.end(), client) != clients.end())
    {
      // hack... client may have been removed...
      packet.clear();
      packet.resize(0);
    }

  TRACE_EXIT();
}

void
DistributionSocketLink::forward_packet_except(PacketBuffer &packet, Client *client, Client *source)
{
  TRACE_ENTER("DistributionSocketLink::forward_packet_except");

  packet.restart_read();
  int flags = packet.peek_byte(3);
  if (!(flags & PACKETFLAG_SOURCE) && source->id != nullptr)
    {
      TRACE_MSG("Add source " << source->id);
      packet.poke_byte(3, flags | PACKETFLAG_SOURCE);
      packet.insert(4, strlen(source->id) + 2);
      packet.poke_string(6, source->id);
    }
  send_packet_except(packet, client);

  TRACE_EXIT();
}

void
DistributionSocketLink::forward_packet(PacketBuffer &packet, Client *dest, Client *source)
{
  TRACE_ENTER("DistributionSocketLink::forward_packet");

  packet.restart_read();
  int flags = packet.peek_byte(3);
  if (!(flags & PACKETFLAG_SOURCE) && source->id != nullptr)
    {
      TRACE_MSG("Add source " << source->id);
      packet.poke_byte(3, flags | PACKETFLAG_SOURCE);
      packet.insert(4, strlen(source->id) + 2);
      packet.poke_string(6, source->id);
    }
  send_packet(dest, packet);
  TRACE_EXIT();
}

string
DistributionSocketLink::get_random_string() const
{
  static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string rnd;

  for (int i = 0; i < 100; ++i)
    {
      rnd += alphanum[g_random_int_range(0, sizeof(alphanum) - 1)];
    }

  return rnd;
}

//! Sends a hello to the specified client.
void
DistributionSocketLink::send_hello1(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_hello1");

  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_HELLO1);

  string rnd = get_random_string();
  client->challenge = rnd;

  TRACE_MSG(username << " " << get_my_id() << " " << rnd);

  packet.pack_string(username);
  packet.pack_string(get_my_id());
  packet.pack_string(rnd);

  send_packet(client, packet);
  TRACE_EXIT();
}

//! Handles a Hello from the specified client.
void
DistributionSocketLink::handle_hello1(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_hello1");

  gchar *user = packet.unpack_string();
  gchar *id = packet.unpack_string();
  gchar *rnd = packet.unpack_string();

  TRACE_MSG(user << " " << id << " " << rnd);

  dist_manager->log(_("Client %s saying hello."), id != nullptr ? id : "Unknown");

  if (user != nullptr && (username == nullptr || (strcmp(username, user) == 0)))
    {
      send_hello2(client, rnd);
    }
  else
    {
      // Incorrect user.
      dist_manager->log(_("Client %s access denied."), id != nullptr ? id : "Unknown");
      remove_client(client);
    }

  g_free(user);
  g_free(id);
  g_free(rnd);

  TRACE_EXIT();
}

//! Sends a hello to the specified client.
void
DistributionSocketLink::send_hello2(Client *client, gchar *rnd)
{
  TRACE_ENTER_MSG("DistributionSocketLink::send_hello2", username << " " << rnd << " " << get_my_id());

  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_HELLO2);

  GHmac *hmac = g_hmac_new(G_CHECKSUM_SHA1, (const guchar *)password, strlen(password));

  g_hmac_update(hmac, (const guchar *)username, strlen(username));
  g_hmac_update(hmac, (const guchar *)rnd, strlen(rnd));
  g_hmac_update(hmac, (const guchar *)get_my_id().c_str(), get_my_id().length());

  packet.pack_string(username);
  packet.pack_string(g_hmac_get_string(hmac));
  packet.pack_string(get_my_id());

  g_hmac_unref(hmac);

  send_packet(client, packet);
  TRACE_EXIT();
}

//! Handles a Hello from the specified client.
void
DistributionSocketLink::handle_hello2(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_hello2");

  gchar *user = packet.unpack_string();
  gchar *pass = packet.unpack_string();
  gchar *id = packet.unpack_string();

  TRACE_MSG(user << " " << pass << " " << id << " " << client->challenge);

  dist_manager->log(_("Client %s saying hello."), id != nullptr ? id : "Unknown");

  GHmac *hmac = g_hmac_new(G_CHECKSUM_SHA1, (const guchar *)password, strlen(password));
  g_hmac_update(hmac, (const guchar *)username, strlen(username));
  g_hmac_update(hmac, (const guchar *)client->challenge.c_str(), client->challenge.length());
  g_hmac_update(hmac, (const guchar *)id, strlen(id));

  if (user != nullptr && pass != nullptr && (username == nullptr || (strcmp(username, user) == 0))
      && (password == nullptr || (strcmp(g_hmac_get_string(hmac), pass) == 0)))
    {
      bool ok = set_client_id(client, id);

      if (ok)
        {
          // Welcome!
          send_welcome(client);
          client->welcome = true;
        }
      else
        {
          // Duplicate client. inform client that it's bogus and close.
          dist_manager->log(_("Client %s is duplicate."), id != nullptr ? id : "Unknown");

          send_duplicate(client);
          remove_client(client);
        }
    }
  else
    {
      // Incorrect password.
      dist_manager->log(_("Client %s access denied."), id != nullptr ? id : "Unknown");
      remove_client(client);
    }

  g_free(user);
  g_free(id);
  g_free(pass);

  g_hmac_unref(hmac);

  TRACE_EXIT();
}

//! Sends a hello to the specified client.
void
DistributionSocketLink::send_signoff(Client *to, Client *signedoff_client)
{
  TRACE_ENTER("DistributionSocketLink::send_signoff");

  PacketBuffer packet;

  packet.create();
  init_packet(packet, PACKET_SIGNOFF);

  if (signedoff_client != nullptr)
    {
      TRACE_MSG("remote client " << (signedoff_client->id != nullptr ? signedoff_client->id : "?"));
      packet.pack_string(signedoff_client->id);
    }
  else
    {
      TRACE_MSG("me " << my_id.str());
      packet.pack_string(get_my_id());
    }

  if (to != nullptr)
    {
      TRACE_MSG("sending to " << (to->id != nullptr ? to->id : "?"));
      send_packet(to, packet);
    }
  else
    {
      TRACE_MSG("broadcasting");
      send_packet_broadcast(packet);
    }
  TRACE_EXIT();
}

//! Handles a Hello from the specified client.
void
DistributionSocketLink::handle_signoff(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_signoff");

  if (!client->welcome)
    {
      return;
    }

  gchar *id = packet.unpack_string();
  Client *c = nullptr;

  if (id != nullptr)
    {
      c = find_client_by_id(id);
      g_free(id);
    }

  if (c != nullptr)
    {
      dist_manager->log(_("Client %s signed off."), c->id == nullptr ? "Unknown" : c->id);

      if (c->type == CLIENTTYPE_DIRECT)
        {
          TRACE_MSG("Direct connection. setting signedoff");
          c->type = CLIENTTYPE_SIGNEDOFF;
          remove_peer_clients(c);

          if (c->socket != nullptr)
            {
              TRACE_MSG("Remove connection");
              delete c->socket;
              c->socket = nullptr;
            }

          remove_client(c);
        }
      else
        {
          TRACE_MSG("Routed connection. removing");
          remove_client(c);
        }
    }

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
DistributionSocketLink::handle_duplicate(PacketBuffer &packet, Client *client)
{
  (void)packet;
  TRACE_ENTER("DistributionSocketLink::handle_duplicate");
  dist_manager->log(_("Client %s is duplicate."), client->id == nullptr ? "Unknown" : client->id);
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
  packet.pack_string(get_my_id());
  packet.pack_string(get_my_id()); // was: hostname
  packet.pack_ushort(server_port);

  send_packet(client, packet);
  TRACE_EXIT();
}

//! Handles a welcome message from the specified client.
void
DistributionSocketLink::handle_welcome(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_welcome");

  gchar *id = packet.unpack_string();
  gchar *name = packet.unpack_string();
  /*gint port = */ packet.unpack_ushort();

  dist_manager->log(_("Client %s is welcoming us."), id == nullptr ? "Unknown" : id);

  bool ok = set_client_id(client, id);

  if (ok)
    {
      client->welcome = true;

      // The connected client offers the master client.
      // This info will be received in the client list.
      // So, we no longer know who's master...
      set_master(nullptr);

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

  g_free(id);
  g_free(name);

  TRACE_EXIT();
}

//! Sends the list of known clients to the specified client.
void
DistributionSocketLink::send_client_list(Client *client, bool except)
{
  TRACE_ENTER("DistributionSocketLink::send_client_list");

  if (clients.size() > 0)
    {
      PacketBuffer packet;
      packet.create();
      init_packet(packet, PACKET_CLIENT_LIST);

      int count = 1;
      gint clients_pos = packet.bytes_written();

      packet.pack_ushort(0); // number of clients in the list
      packet.pack_ushort(0); // flags.

      // Put muself in list.
      gint pos = packet.bytes_written();

      TRACE_MSG("client me: " << my_id.str() << " " << server_port << " " << i_am_master);
      int flags = CLIENTLIST_ME | (i_am_master ? CLIENTLIST_MASTER : 0);
      packet.pack_ushort(0);           // Length
      packet.pack_ushort(flags);       // Flags
      packet.pack_string(get_my_id()); // ID
      packet.pack_string(get_my_id()); // Canonical name
      packet.pack_ushort(server_port); // Listen port.

      // Size of the client data.
      packet.poke_ushort(pos, packet.bytes_written() - pos);

      // Put known client in the list.
      list<Client *>::iterator i = clients.begin();
      while (i != clients.end())
        {
          Client *c = *i;

          if (c->id != nullptr)
            {
              count++;
              pos = packet.bytes_written();

              int flags = 0;
              if (c == master_client)
                {
                  flags |= CLIENTLIST_MASTER;
                }

              TRACE_MSG("Send client: " << c->id);

              packet.pack_ushort(0);           // Length
              packet.pack_ushort(flags);       // Flags
              packet.pack_string(c->id);       // ID
              packet.pack_string(c->hostname); // Canonical name
              packet.pack_ushort(c->port);     // Listen port.

              // Size of the client data.
              packet.poke_ushort(pos, packet.bytes_written() - pos);
            }
          i++;
        }

      // Put packet size in the packet and send.
      packet.poke_ushort(clients_pos, count);

      if (except)
        {
          send_packet_except(packet, client);
        }
      else
        {
          send_packet(client, packet);
        }
    }

  TRACE_EXIT();
}

//! Handles a client list from the specified client.
bool
DistributionSocketLink::handle_client_list(PacketBuffer &packet, Client *client, Client *direct)
{
  TRACE_ENTER("DistributionSocketLink::handle_client_list");

  if (!client->welcome)
    {
      TRACE_EXIT();
      return false;
    }

  // Extract data.
  gint num_clients = packet.unpack_ushort();
  gint flags = packet.unpack_ushort();
  (void)flags;

  gchar *master_id = nullptr;

  gchar **names = new gchar *[num_clients];
  gchar **ids = new gchar *[num_clients];
  gint *ports = new gint[num_clients];

  bool ok = true;

  // Loop over remote clients.
  for (int i = 0; i < num_clients; i++)
    {
      names[i] = nullptr;
      ids[i] = nullptr;
      ports[i] = 0;

      // Extract data.
      gint pos = packet.bytes_read();
      gint size = packet.unpack_ushort();
      gint flags = packet.unpack_ushort();
      gchar *id = packet.unpack_string();
      gchar *name = packet.unpack_string();
      gint port = packet.unpack_ushort();

      if (flags & CLIENTLIST_MASTER)
        {
          master_id = g_strdup(id);
          TRACE_MSG("Master: " << master_id);
        }

      if (id != nullptr)
        {
          if (!exists_client(id))
            {
              // A new client
              TRACE_MSG("new client: " << id);
              names[i] = name;
              ids[i] = id;
              ports[i] = port;
            }
          else if (client != nullptr && direct == client && !client_is_me(id) && strcmp(client->id, id) != 0)
            {
              TRACE_MSG("Strange client: " << id);
              ok = false;
            }
        }

      // Skip trailing junk...
      size -= (packet.bytes_read() - pos);
      packet.skip(size);

      g_free(name);
      g_free(id);
    }

  if (ok)
    {
      // And send the list of client we are connected to.
      if (client != nullptr && direct == client && !client->sent_client_list)
        {
          client->sent_client_list = true;
          send_client_list(client);
        }

      TRACE_MSG("Adding: ");
      for (int i = 0; i < num_clients; i++)
        {
          if (ids[i] != nullptr && names[i] != nullptr)
            {
              add_client(ids[i], names[i], ports[i], CLIENTTYPE_ROUTED, direct);
            }
        }

      if (master_id != nullptr)
        {
          set_master_by_id(master_id);
          TRACE_MSG(master_id << " is now master");
        }

      send_client_message(DCMT_SIGNON);
    }
  else
    {
      TRACE_MSG("Dup: ");
      dist_manager->log(_("Client %s is duplicate."), client->id != nullptr ? client->id : "Unknown");

      send_duplicate(client);
      remove_client(client);
    }

  TRACE_MSG("Ok: ");

  for (int i = 0; i < num_clients; i++)
    {
      g_free(ids[i]);
      g_free(names[i]);
    }

  g_free(master_id);
  delete[] names;
  delete[] ids;
  delete[] ports;

  TRACE_EXIT();
  return ok;
}

//! Requests to become master.
void
DistributionSocketLink::send_claim(Client *client)
{
  TRACE_ENTER("DistributionSocketLink::send_claim");

  if (client->next_claim_time == 0 || time(nullptr) >= client->next_claim_time)
    {
      PacketBuffer packet;

      dist_manager->log(_("Requesting master status from %s."), client->id == nullptr ? "Unknown" : client->id);

      packet.create();
      init_packet(packet, PACKET_CLAIM);

      packet.pack_ushort(0);

      client->next_claim_time = time(nullptr) + 10;

      send_packet(client, packet);

      if (client->claim_count >= 3)
        {
          dist_manager->log(_("Client timeout from %s."), client->id == nullptr ? "Unknown" : client->id);

          close_client(client, client->outbound);
        }
      client->claim_count++;
    }

  TRACE_EXIT();
}

//! Handles a request from a remote client to become master.
void
DistributionSocketLink::handle_claim(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");

  if (!client->welcome)
    {
      TRACE_EXIT();
      return;
    }

  /*gint count = */ packet.unpack_ushort();

  if (i_am_master && master_locked)
    {
      dist_manager->log(_("Rejecting master request from client %s."), client->id == nullptr ? "Unknown" : client->id);
      send_claim_reject(client);
    }
  else
    {
      dist_manager->log(_("Acknowledging master request from client %s."), client->id == nullptr ? "Unknown" : client->id);

      bool was_master = i_am_master;

      // Marks client as master
      set_master(client);
      assert(!i_am_master);

      // If I was previously master, distribute state.
      if (was_master)
        {
          // dist_manager->log(_("Transferring state to client %s:%d."),
          //                  client->hostname, client->port);
          send_client_message(DCMT_MASTER);
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
DistributionSocketLink::handle_claim_reject(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_claim");
  (void)packet;

  if (!client->welcome)
    {
      TRACE_EXIT();
      return;
    }

  if (client != master_client)
    {
      dist_manager->log(_("Non-master client %s rejected master request."), client->id == nullptr ? "Unknown" : client->id);
    }
  else
    {
      dist_manager->log(_("Client %s rejected master request, delaying."), client->id == nullptr ? "Unknown" : client->id);
      client->reject_count++;
      int count = client->reject_count;

      if (count > 6)
        {
          count = 6;
        }

      client->next_claim_time = time(nullptr) + 5 * count;
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

  string id;

  if (master_client == nullptr)
    {
      // I've become master
      id = get_my_id();
    }
  else if (master_client->id != nullptr)
    {
      // Another remote client becomes master
      id = master_client->id;
    }

  packet.pack_string(id);
  packet.pack_ushort(0);

  if (client != nullptr)
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
DistributionSocketLink::handle_new_master(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink::handle_new_master");

  if (!client->welcome)
    {
      TRACE_EXIT();
      return;
    }

  for (list<Client *>::iterator i = clients.begin(); i != clients.end(); i++)
    {
      (*i)->reject_count = 0;
    }

  gchar *id = packet.unpack_string();
  /* gint count = */ packet.unpack_ushort();

  dist_manager->log(_("Client %s is now the new master."), id == nullptr ? "Unknown" : id);

  if (client->id != nullptr)
    {
      TRACE_MSG("new master from " << client->id << " -> " << id);
    }

  set_master_by_id(id);

  g_free(id);

  TRACE_EXIT();
}

// Distributes the current client message.
void
DistributionSocketLink::send_client_message(DistributionClientMessageType type)
{
  TRACE_ENTER("DistributionSocketLink:send_client_message");

  PacketBuffer packet;
  packet.create();
  init_packet(packet, PACKET_CLIENTMSG);

  string id = get_master();
  packet.pack_string(id);

  packet.pack_ushort(client_message_map.size());

  ClientMessageMap::iterator i = client_message_map.begin();
  while (i != client_message_map.end())
    {
      DistributionClientMessageID id = i->first;
      ClientMessageListener &sl = i->second;

      IDistributionClientMessage *itf = sl.listener;

      int pos = 0;
      packet.pack_ushort(id);
      packet.reserve_size(pos);

      if ((sl.type & type) != 0)
        {
          TRACE_MSG("request " << id << " " << type);
          itf->request_client_message(id, packet);
        }

      packet.update_size(pos);

      i++;
    }

  send_packet_broadcast(packet);
  TRACE_EXIT();
}

//! Handles client message  from a remote client.
void
DistributionSocketLink::handle_client_message(PacketBuffer &packet, Client *client)
{
  TRACE_ENTER("DistributionSocketLink:handle_client_message");

  if (!client->welcome)
    {
      TRACE_EXIT();
      return;
    }

  bool will_i_become_master = false;

  // dist_manager->log(_("Reveived client message from client %s:%d."), client->hostname, client->port);

  gchar *id = packet.unpack_string();

  if (id != nullptr)
    {
      // TRACE_MSG("id = " << id);

      will_i_become_master = client_is_me(id);
      g_free(id);
    }

  gint size = packet.unpack_ushort();
  int pos;

  TRACE_MSG("size = " << size);
  for (int i = 0; i < size; i++)
    {
      DistributionClientMessageID id = (DistributionClientMessageID)packet.unpack_ushort();
      gint datalen = packet.read_size(pos);

      TRACE_MSG("len = " << datalen << " " << id);

      if (datalen != 0)
        {
          // Narrow the buffer to the client message data.
          packet.narrow(-1, datalen);

          ClientMessageMap::iterator it = client_message_map.find(id);
          if (it != client_message_map.end())
            {
              client_message_map[id].listener->client_message(id, will_i_become_master, client->id, packet);
            }

          packet.narrow(0, -1);
        }

      packet.skip_size(pos);
    }

  TRACE_EXIT();
}

bool
DistributionSocketLink::start_async_server()
{
  TRACE_ENTER("DistributionSocketLink::start_async_server");
  bool ret = false;

  try
    {
      /* Create the server */
      server_socket = socket_driver->create_server();

      if (server_socket != nullptr)
        {
          server_socket->set_listener(this);
          server_socket->listen(server_port);
          dist_manager->log(_("Network operation started."));
          ret = true;
        }
    }
  catch (SocketException &e)
    {
    }

  TRACE_RETURN(ret);
  return ret;
}

void
DistributionSocketLink::socket_accepted(ISocketServer *scon, ISocket *ccon)
{
  (void)scon;

  TRACE_ENTER("DistributionSocketLink::socket_accepted");
  if (ccon != nullptr)
    {
      dist_manager->log(_("Accepted new client."));

      Client *client = new Client;
      client->type = CLIENTTYPE_DIRECT;
      client->peer = nullptr;
      client->packet.create();

      TRACE_RETURN(client->packet.bytes_available());
      client->socket = ccon;
      client->hostname = nullptr;
      client->id = nullptr;
      client->port = 0;
      client->reconnect_count = 0;
      client->reconnect_time = 0;

      ccon->set_data(client);
      ccon->set_listener(this);
      clients.push_back(client);

      send_hello1(client);
    }
  TRACE_EXIT();
}

void
DistributionSocketLink::socket_io(ISocket *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_io");
  bool ret = true;

  Client *client = (Client *)data;
  g_assert(client != nullptr);

  TRACE_MSG("1");

  if (!is_client_valid(client) && client->type == CLIENTTYPE_DIRECT)
    {
      TRACE_RETURN("Invalid client");
      return;
    }

  int bytes_read = 0;
  int bytes_to_read = 4;

  TRACE_MSG("2 " << client->packet.bytes_available());

  if (client->packet.bytes_available() >= 4)
    {
      TRACE_MSG("3");

      bytes_to_read = client->packet.peek_ushort(0) - 4;

      TRACE_MSG("4 " << bytes_to_read);

      if (bytes_to_read + 4 > client->packet.get_buffer_size())
        {
          TRACE_MSG("5 " << bytes_to_read << " " << client->packet.get_buffer_size());
          // FIXME: the 1024 is lame...
          client->packet.resize(bytes_to_read + 4 + 1024);
        }
    }

  TRACE_MSG("5");
  bool ok = true;
  try
    {
      con->read(client->packet.get_write_ptr(), bytes_to_read, bytes_read);
    }
  catch (SocketException &)
    {
      ok = false;
    }

  if (!ok)
    {
      dist_manager->log(_("Client %s read error, closing."), client->id == nullptr ? "Unknown" : client->id);
      ret = false;
    }
  else if (bytes_read == 0)
    {
      dist_manager->log(_("Client %s closed connection."), client->id == nullptr ? "Unknown" : client->id);
      ret = false;
    }
  else
    {
      g_assert(bytes_read > 0);
      client->packet.write_ptr += bytes_read;

      if (client->packet.peek_ushort(0) == client->packet.bytes_written())
        {
          process_client_packet(client);
        }
    }

  if (!ret)
    {
      close_client(client, client->outbound);
    }

  TRACE_EXIT();
  return;
}

void
DistributionSocketLink::socket_connected(ISocket *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_connected");

  Client *client = (Client *)data;

  g_assert(client != nullptr);
  g_assert(con != nullptr);

  if (!is_client_valid(client) && client->type == CLIENTTYPE_DIRECT)
    {
      TRACE_RETURN("Invalid client");
      return;
    }

  dist_manager->log(_("Client %s connected."), client->id != nullptr ? client->id : "Unknown");

  client->reconnect_count = 0;
  client->reconnect_time = 0;
  client->outbound = true;
  client->socket = con;

  TRACE_EXIT();
}

void
DistributionSocketLink::socket_closed(ISocket *con, void *data)
{
  TRACE_ENTER("DistributionSocketLink::socket_closed");
  (void)con;

  Client *client = (Client *)data;
  assert(client != nullptr);

  if (!is_client_valid(client) && client->type == CLIENTTYPE_DIRECT)
    {
      TRACE_RETURN("Invalid client");
      return;
    }

  // Socket error. Disable client.
  if (client->socket != nullptr)
    {
      dist_manager->log(_("Client %s closed connection."), client->id != nullptr ? client->id : "Unknown");
      close_client(client, client->outbound);
    }
  else
    {
      dist_manager->log(_("Could not connect to client %s."), client->id != nullptr ? client->id : "Unknown");
      remove_client(client);
    }

  TRACE_EXIT();
}

//! Read the configuration from the configurator.
void
DistributionSocketLink::read_configuration()
{
  TRACE_ENTER("DistributionSocketLink::read_configuration");
  int old_port = server_port;

  const char *port = getenv("WORKRAVE_PORT");
  if (port != nullptr)
    {
      server_port = atoi(port);
    }
  else
    {
      server_port = dist_manager->get_port();
    }

  if (old_port != server_port && server_enabled)
    {
      set_server_enabled(false);
      set_server_enabled(true);
    }

  reconnect_interval = dist_manager->get_reconnect_interval();
  reconnect_attempts = dist_manager->get_reconnect_attempts();

  string str;
  str = dist_manager->get_username();
  username = str != "" ? g_strdup(str.c_str()) : nullptr;

  str = dist_manager->get_password();
  password = str != "" ? g_strdup(str.c_str()) : nullptr;
  TRACE_EXIT();
}

//! Notification from the configurator that the configuration has changed.
void
DistributionSocketLink::config_changed_notify(const string &key)
{
  (void)key;
  read_configuration();
}

#endif
