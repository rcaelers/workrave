// DistributionSocketLink.hh
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
// $Id$
//

#ifndef DISTRIBUTIONSOCKETLINK_HH
#define DISTRIBUTIONSOCKETLINK_HH

#include <list>
#include <map>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "DistributionLink.hh"
#include "DistributedStateInterface.hh"
#include "ConfiguratorListener.hh"
#include "PacketBuffer.hh"

#include "SocketDriver.hh"

#define DEFAULT_PORT (27273)
#define DEFAULT_INTERVAL (15)
#define DEFAULT_ATTEMPTS (5)

class DistributionLinkListener;
class Configurator;

class DistributionSocketLink :
  public DistributionLink,
  public ConfiguratorListener,
  public SocketListener
{
public:
  static const string CFG_KEY_DISTRIBUTION_TCP;
  static const string CFG_KEY_DISTRIBUTION_TCP_PORT;
  static const string CFG_KEY_DISTRIBUTION_TCP_USERNAME;
  static const string CFG_KEY_DISTRIBUTION_TCP_PASSWORD;
  static const string CFG_KEY_DISTRIBUTION_TCP_INTERVAL;
  static const string CFG_KEY_DISTRIBUTION_TCP_ATTEMPTS;
  
private:
  enum PacketCommand {
    PACKET_HELLO 	= 0x0001,
    PACKET_CLAIM 	= 0x0002,
    PACKET_CLIENT_LIST	= 0x0003,
    PACKET_WELCOME	= 0x0004,
    PACKET_NEW_MASTER	= 0x0005,
    PACKET_STATEINFO	= 0x0006,
    PACKET_DUPLICATE	= 0x0007,
    PACKET_CLAIM_REJECT	= 0x0008,
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
      socket(NULL),
      hostname(NULL),
      port(0),
      reconnect_count(0),
      reconnect_time(0),
      next_claim_time(0),
      reject_count(0),
      claim_count(0)
    {
    }
    
    ~Client()
    {
      if (socket != NULL)
        {
          delete socket;
        }
      if (hostname != NULL)
        {
          g_free(hostname);
        }
    }

    //!
    SocketConnection *socket;
    
    //! Canonical IP.
    gchar *hostname;
    
    //! port.
    gint port;

    //!
    PacketBuffer packet;

    //! Reconnect counter;
    int reconnect_count;
    
    //! Last reconnect attempt time;
    time_t reconnect_time;

    //! Next time we can try to claim from this client;
    time_t next_claim_time;

    //! Number of time a claim was rejected.
    int reject_count;

    //! Number of claims send since the last received packet.
    int claim_count;
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
  bool disconnect_all();
  bool reconnect_all();
  bool claim();
  bool set_lock_master(bool lock);

  bool register_state(DistributedStateID id, DistributedStateInterface *dist_state);
  bool unregister_state(DistributedStateID id);
  bool push_state(DistributedStateID id, unsigned char *buffer, int size);

  void socket_accepted(SocketConnection *scon, SocketConnection *ccon);
  void socket_connected(SocketConnection *con, void *data);
  void socket_io(SocketConnection *con, void *data);
  void socket_closed(SocketConnection *con, void *data);
  
private:
  bool is_client_valid(Client *client);
  bool add_client(gchar *host, gint port);
  bool remove_client(Client *client);
  Client *find_client_by_canonicalname(gchar *name, gint port);
  bool client_is_me(gchar *host, gint port);
  bool exists_client(gchar *host, gint port);
  bool set_canonical(Client *client, gchar *host, gint port);

  bool get_master(gchar **name, gint *port) const;
  void set_master(gchar *cname, gint port);
  void set_master(Client *client);
  void set_me_master();

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
  void handle_claim_reject(Client *client);
  void send_hello(Client *client);
  void send_welcome(Client *client);
  void send_duplicate(Client *client);
  void send_client_list(Client *client);
  void send_claim(Client *client);
  void send_new_master(Client *client = NULL);
  void send_claim_reject(Client *client);
  void send_state();
  
  bool start_async_server();

  void read_configuration();
  void config_changed_notify(string key);
  
private:
  //! The socket library.
  SocketDriver *socket_driver;
  
  //! The distribution manager
  DistributionLinkListener *dist_manager;

  //! The configuration access.
  Configurator *configurator;
  
  //! Username for client authenication
  gchar *username;

  //! Password for client authenication.
  gchar *password;

  //! All clients.
  list<Client *> clients;

  //! Active client
  Client *master_client;

  //!
  bool i_am_master;

  //!
  bool master_locked;
  
  //!
  gchar *myname;
  
  //! My server port
  gint server_port;
  
  //! The server socket.
  SocketConnection *server_socket;

  //!
  bool server_enabled;
  
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
