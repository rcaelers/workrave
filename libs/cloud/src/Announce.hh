// Announce.hh --- ing network server
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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
//

#ifndef ANNOUNCE_HH
#define ANNOUNCE_HH

#include <string>
#include <set>
#include <boost/shared_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "network/MulticastSocketServer.hh"

#include "IRouter.hh"
#include "Marshaller.hh"
#include "Link.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::cloud;

class Announce
{
public:
  typedef boost::shared_ptr<Announce> Ptr;

public:
  static Ptr create(IRouter::Ptr router, Marshaller::Ptr marshaller);

  Announce(IRouter::Ptr router, Marshaller::Ptr marshaller);
  virtual ~Announce();

  void init(int announce_port, UUID &id, int direct_link_port);
  void terminate();
  void heartbeat();
  void start();

  void send_message(const std::string &message);

  typedef boost::signals2::signal<bool(Link::Ptr, PacketIn::Ptr)> data_signal_type;
  
  data_signal_type &signal_data();
 
private:
  enum AnnounceState {
    ANNOUNCE_STATE_HOLD,
    ANNOUNCE_STATE_MONITORING,
    ANNOUNCE_STATE_DISCOVER,
    ANNOUNCE_STATE_BUILD,
  };

  void on_data(gsize size, const gchar *data, NetworkAddress::Ptr na);

  void send_discover();
  void send_routing_data();

  void send_message(Message::Ptr message, MessageParams::Ptr params);
  
  void process_discover(EphemeralLink::Ptr link, PacketIn::Ptr packet);
  void process_routing_data(EphemeralLink::Ptr link, PacketIn::Ptr packet);
  void connect_to_clients();
  
  void goto_monitoring();
  void goto_hold();
  void goto_discover(bool immediate);
  void goto_build();

  static gboolean static_on_timer(gpointer data);

private:
  typedef std::set<UUID> UUIDSet;
  typedef UUIDSet::iterator UUIDSetIter;


  class RemoteClient
  {
  public:
    NetworkAddress::Ptr address;
    int port;
    UUIDSet direct_connections;
    UUIDSet all_connections;
  };

  typedef std::map<UUID, RemoteClient> RemoteClientMap;
  typedef RemoteClientMap::iterator RemoteClientMapIter;
  typedef RemoteClientMap::const_iterator RemoteClientMapCIter;
  
private:

  //!
  IRouter::Ptr router;
  
  //!
  Marshaller::Ptr marshaller;
  
  //! Default server
  MulticastSocketServer::Ptr multicast_server;

  //!
  data_signal_type data_signal;

  AnnounceState state;
  
  gint64 wait_until_time;
  int announce_left;

  UUID myid;

  RemoteClientMap remote_clients;

  int direct_link_port;
};


#endif // ANNOUNCE_HH
