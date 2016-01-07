// DistributionSocketLink.hh
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008, 2010 Rob Caelers <robc@krandor.org>
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
#include "IDistributionClientMessage.hh"
#include "IConfiguratorListener.hh"
#include "PacketBuffer.hh"

#include "SocketDriver.hh"
#include "WRID.hh"

#define DEFAULT_PORT (27273)
#define DEFAULT_INTERVAL (15)
#define DEFAULT_ATTEMPTS (5)

class Configurator;

class DistributionSocketLink :
  public DistributionLink,
  public IConfiguratorListener,
  public ISocketServerListener,
  public ISocketListener
{
public:
private:
  enum PacketCommand {
    PACKET_HELLO1       = 0x0001,
    PACKET_CLAIM        = 0x0002,
    PACKET_CLIENT_LIST  = 0x0003,
    PACKET_WELCOME      = 0x0004,
    PACKET_NEW_MASTER   = 0x0005,
    PACKET_CLIENTMSG    = 0x0006,
    PACKET_DUPLICATE    = 0x0007,
    PACKET_CLAIM_REJECT = 0x0008,
    PACKET_SIGNOFF      = 0x0009,
    PACKET_HELLO2       = 0x000A,
  };

  enum PacketFlags {
    PACKETFLAG_SOURCE   = 0x0001,
    PACKETFLAG_DEST     = 0x0002,
  };

  enum ClientListFlags
    {
      CLIENTLIST_ME     = 1,
      CLIENTLIST_MASTER = 2,
    };

  struct ClientMessageListener
  {
    IDistributionClientMessage *listener;
    DistributionClientMessageType type;

    ClientMessageListener() :
      listener(NULL),
      type(DCMT_PASSIVE)
    {
    }
  };

  enum ClientType
    {
      CLIENTTYPE_UNKNOWN    = 1,
      CLIENTTYPE_DIRECT     = 2,
      CLIENTTYPE_ROUTED     = 3,
      CLIENTTYPE_SIGNEDOFF  = 4,
    };

  struct Client
  {
    Client() :
      type(CLIENTTYPE_UNKNOWN),
      peer(NULL),
      socket(NULL),
      id(NULL),
      hostname(NULL),
      port(0),
      sent_client_list(false),
      welcome(false),
      reconnect_count(0),
      reconnect_time(0),
      next_claim_time(0),
      reject_count(0),
      claim_count(0),
      outbound(false)
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
      hostname = NULL;
    }

    //! Type of connection with client.
    ClientType type;

    //! Peer client for remote clients.
    Client *peer;

    //!
    ISocket *socket;

    //! ID
    gchar *id;

    //! Challenge
    std::string challenge;
    
    //! Canonical IP.
    gchar *hostname;

    //! port.
    gint port;

    //!
    bool sent_client_list;

    //!
    bool welcome;

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

    //! Is this an outbound connection
    bool outbound;
  };


public:
  DistributionSocketLink(Configurator *conf);
  virtual ~DistributionSocketLink();

  void init_my_id();
  std::string get_my_id() const;
  int get_number_of_peers();
  void set_distribution_manager(DistributionManager *dll);
  void init();
  void heartbeat();
  bool set_network_enabled(bool enabled);
  bool set_server_enabled(bool enabled);
  void set_user(string user, string password);
  void connect(string url);
  void disconnect(string id);
  bool disconnect_all();
  bool reconnect_all();
  bool claim();
  bool set_lock_master(bool lock);

  bool register_client_message(DistributionClientMessageID id, DistributionClientMessageType type,
                               IDistributionClientMessage *callback);
  bool unregister_client_message(DistributionClientMessageID id);
  bool broadcast_client_message(DistributionClientMessageID id, PacketBuffer &buffer);

  void socket_accepted(ISocketServer *server, ISocket *con);
  void socket_connected(ISocket *con, void *data);
  void socket_io(ISocket *con, void *data);
  void socket_closed(ISocket *con, void *data);

private:
  bool is_client_valid(Client *client);
  bool add_client(gchar *id, gchar *host, gint port, ClientType type, Client *peer = NULL);
  void remove_client(Client *client);
  void remove_peer_clients(Client *client);
  void close_client(Client *client, bool reconnect = false);
  Client *find_client_by_canonicalname(gchar *name, gint port);
  Client *find_client_by_id(gchar *id);
  bool client_is_me(gchar *id);
  bool exists_client(gchar *id);

  bool set_client_id(Client *client, gchar *id);

  string get_master() const;
  void set_master_by_id(gchar *id);
  void set_master(Client *client);
  void set_me_master();

  void init_packet(PacketBuffer &packet, PacketCommand cmd);
  void send_packet_broadcast(PacketBuffer &packet);
  void send_packet_except(PacketBuffer &packet, Client *client);
  void send_packet(Client *client, PacketBuffer &packet);
  void forward_packet_except(PacketBuffer &packet, Client *client, Client *source);
  void forward_packet(PacketBuffer &packet, Client *dest, Client *source);

  void process_client_packet(Client *client);
  void handle_hello1(PacketBuffer &packet, Client *client);
  void handle_hello2(PacketBuffer &packet, Client *client);
  void handle_signoff(PacketBuffer &packet, Client *client);
  void handle_welcome(PacketBuffer &packet, Client *client);
  void handle_duplicate(PacketBuffer &packet, Client *client);
  bool handle_client_list(PacketBuffer &packet, Client *client, Client *direct);
  void handle_claim(PacketBuffer &packet, Client *client);
  void handle_new_master(PacketBuffer &packet, Client *client);
  void handle_client_message(PacketBuffer &packet, Client *client);
  void handle_claim_reject(PacketBuffer &packet, Client *client);

  void send_hello1(Client *client);
  void send_hello2(Client *client, gchar *rnd);
  void send_signoff(Client *to, Client *signedoff_client);
  void send_welcome(Client *client);
  void send_duplicate(Client *client);
  void send_client_list(Client *client, bool except = false);
  void send_claim(Client *client);
  void send_new_master(Client *client = NULL);
  void send_claim_reject(Client *client);
  void send_client_message(DistributionClientMessageType type);

  bool start_async_server();

  void read_configuration();
  void config_changed_notify(const string &key);

  std::string get_random_string() const;
  
private:
  typedef map<DistributionClientMessageID, ClientMessageListener> ClientMessageMap;

  //! The distribution manager.
  DistributionManager *dist_manager;

  SocketDriver *socket_driver;

  //! The configuration access.
  Configurator *configurator;

  //! My ID
  WRID my_id;

  //! Username for client authentication
  gchar *username;

  //! Password for client authentication.
  gchar *password;

  //! All clients.
  list<Client *> clients;

  //! Active client
  Client *master_client;

  //! Whether I'm the master.
  bool i_am_master;

  //! Whether the master status is locked by me.
  bool master_locked;

  //! My name
  //gchar *myname;

  //! My ID accross the network.
  //gchar *myid;

  //! My server port
  gint server_port;

  //! The server socket.
  ISocketServer *server_socket;

  //! Whether distribution is enabled.
  bool network_enabled;
  bool server_enabled;

  //! ClientMessage listeners
  ClientMessageMap client_message_map;

  //!
  int reconnect_attempts;

  //!
  int reconnect_interval;

  //!
  int heartbeat_count;
};

#endif // DISTRIBUTIONSOCKETLINK_HH
