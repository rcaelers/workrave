// DistributionSocketLink.hh
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

#ifndef DISTRIBUTIONSOCKETLINK_HH
#define DISTRIBUTIONSOCKETLINK_HH

#include <list>
#include <map>
#include <glib.h>
#include <gnet/gnet.h>

#include "DistributionLink.hh"
#include "DistributedStateInterface.hh"
#include "ConfiguratorListener.hh"
#include "PacketBuffer.hh"

class DistributionLinkListener;
class Configurator;

class DistributionSocketLink :
  public DistributionLink,
  public ConfiguratorListener
{
private:
  enum PacketCommand {
    PACKET_HELLO 	= 0x0001,
    PACKET_CLAIM 	= 0x0002,
    PACKET_CLIENT_LIST	= 0x0003,
    PACKET_WELCOME	= 0x0004,
    PACKET_NEW_MASTER	= 0x0005,
    PACKET_STATEINFO	= 0x0006,
    PACKET_DUPLICATE	= 0x0007,
  };

  enum ClientListFlags
    {
      CLIENTLIST_FORWARDABLE  		= 1,
      CLIENTLIST_IAM_ACTIVE   		= 2,
      CLIENTLIST_HAS_ACTIVE_REF   	= 4,
    };
  
  struct Client
  {
    Client() :
      canonical_name(NULL),
      server_name(NULL),
      server_port(0),
      socket(NULL),
      iochannel(NULL),
      watch_flags(0),
      watch(0),
      link(NULL),
      reconnect_count(0),
      reconnect_time(0)
    {
    }

    ~Client()
    {
      if (canonical_name != NULL)
        {
          g_free(canonical_name);
        }
      if (server_name != NULL)
        {
          g_free(server_name);
        }

      if (iochannel != NULL)
        {
          g_io_channel_unref(iochannel);
        }

      if (socket != NULL)
        {
          gnet_tcp_socket_delete(socket);
        }
      
      g_source_remove(watch);
    }
    
    //! Canonical IP.
    gchar *canonical_name;
    
    //! Hostname/IP of this client.
    gchar *server_name;

    //! Local port.
    gint server_port;

    //! GNet socket
    GTcpSocket *socket;

    //! Glib IOChannel.
    GIOChannel *iochannel;

    //! I/O Events we are monitoring.
    gint watch_flags;

    //! Our Watch
    guint watch;

    //! For statics...
    DistributionSocketLink *link;

    //!
    PacketBuffer packet;

    //! Reconnect counter;
    int reconnect_count;
    
    //! Last reconnect attempt time;
    time_t reconnect_time;

  };

  
public:
  DistributionSocketLink(Configurator *conf);
  virtual ~DistributionSocketLink();
  
  int get_number_of_peers();
  void set_distribution_manager(DistributionLinkListener *dll);
  bool init();
  void heartbeat();
  bool set_enabled(bool enabled);
  void set_user(string user, string password);
  void join(string url);
  bool claim();

  bool register_state(DistributedStateID id, DistributedStateInterface *dist_state);
  bool unregister_state(DistributedStateID id);
  
private:
  bool add_client(gchar *host, gint port);
  bool remove_client(Client *client);
  Client *find_client_by_servername(gchar *name, gint port);
  Client *find_client_by_canonicalname(gchar *name, gint port);
  bool exits_client(gchar *host, gint port);
  bool set_canonical(Client *client, gchar *host, gint port);

  void set_active(gchar *cname, gint port);
  void set_active(Client *client);
  void set_me_active();

  void init_packet(PacketBuffer &packet, PacketCommand cmd);
  void send_packet_broadcast(PacketBuffer &packet);
  void send_packet_except(PacketBuffer &packet, Client *client);
  void send_packet(Client *client, PacketBuffer &packet);

  void process_client_packet(Client *client);
  void handle_hello(Client *client);
  void handle_welcome(Client *client);
  void handle_duplicate(Client *client);
  void handle_client_list(Client *client);
  void handle_claim(Client *client);
  void handle_new_master(Client *client);
  void handle_state(Client *client);
  void send_hello(Client *client);
  void send_welcome(Client *client);
  void send_duplicate(Client *client);
  void send_client_list(Client *client);
  void send_claim(Client *client);
  void send_new_master(Client *client = NULL);
  void send_state();
  
  bool start_async_server();
  void async_accept(GTcpSocket *server, GTcpSocket *client);
  bool async_io(GIOChannel* iochannel, GIOCondition condition, Client *client);
  void async_connected(GTcpSocket *socket, GInetAddr *ia, GTcpSocketConnectAsyncStatus status, Client *client);

  static void static_async_accept(GTcpSocket* server, GTcpSocket* client, gpointer data);
  static gboolean static_async_io(GIOChannel* iochannel, GIOCondition condition, gpointer data);
  static void static_async_connected(GTcpSocket *socket, GInetAddr *ia, GTcpSocketConnectAsyncStatus status, gpointer data);

  void read_configuration();
  void config_changed_notify(string key);
  
private:
  static const string CFG_KEY_DISTRIBUTION_TCP;
  static const string CFG_KEY_DISTRIBUTION_TCP_PORT;
  static const string CFG_KEY_DISTRIBUTION_TCP_USERNAME;
  static const string CFG_KEY_DISTRIBUTION_TCP_PASSWORD;
  static const string CFG_KEY_DISTRIBUTION_TCP_INTERVAL;
  static const string CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS;

  //! Username for client authenication
  gchar *username;

  //! Password for client authenication.
  gchar *password;

  //! All clients.
  list<Client *> clients;

  //! Active client
  Client *active_client;

  //!
  bool active;
  
  //!
  gchar *canonical_name;
  
  //! My server port
  gint server_port;
  
  //! The server socket.
  GTcpSocket *server_socket;

  //!
  bool server_enabled;
  
  //!
  DistributionLinkListener *dist_manager;

  //!
  Configurator *configurator;
  
  //! State
  map<DistributedStateID, DistributedStateInterface *> state_map;

  //!
  int reconnect_attempts;

  //!
  int reconnect_interval;

  //!
  int heartbeat_count;
};

#endif // DISTRIBUTIONSOCKETLINK_HH
