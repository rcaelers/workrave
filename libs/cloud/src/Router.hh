// Router.hh --- Networking network server
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

#ifndef ROUTER_HH
#define ROUTER_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <google/protobuf/message.h>

#include "cloud/Cloud.hh"

#include "Announce.hh"
#include "DirectLinkManager.hh"
#include "DirectLink.hh"
#include "Client.hh"
#include "IRouter.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::cloud;

class Router : public ICloud, public ICloudTest, public IRouter, public boost::enable_shared_from_this<Router>
{
public:
  typedef boost::shared_ptr<Router> Ptr;

  typedef std::list<Client::Ptr> Clients;
  typedef std::list<Client::Ptr>::iterator ClientIter;
  typedef std::list<Client::Ptr>::const_iterator ClientCIter;
  
public:
  static Ptr create();

public:  
  virtual ~Router();

  virtual void init(int port, std::string username, std::string secret);
  virtual void terminate();
  virtual void heartbeat();
  virtual void connect(const std::string &host, int port);
  virtual void send_message(Message::Ptr message, MessageParams::Ptr params);

  virtual MessageSignal &signal_message(int domain, int id);

  virtual std::list<workrave::cloud::ClientInfo> get_client_infos() const;
  
  virtual UUID get_id() const;
  virtual std::list<UUID> get_clients() const;
  virtual std::list<UUID> get_direct_clients() const;
  virtual int get_cycle_failures() const;
  virtual void start_announce();
  virtual void disconnect(UUID id);

  
private:
  Router();
  void post_construct();
  
  void init_myid(int instanceid);

  bool on_data(Link::Ptr link, PacketIn::Ptr packet, Scope scope);

  void on_direct_link_created(DirectLink::Ptr link);
  void on_direct_link_state_changed(DirectLink::Ptr link);

  void fire_message_signal(int domain, int id, Message::Ptr, MessageContext::Ptr);

  void forward_message(Link::Ptr link, PacketIn::Ptr packet);

  void send_message(Link::Ptr link, Message::Ptr message, MessageParams::Ptr params);
    
  void send_alive(Link::Ptr link = Link::Ptr());
  void send_signoff(std::list<UUID> ids);
  void process_alive(Link::Ptr link, PacketIn::Ptr packet);
  void process_signoff(Link::Ptr link, PacketIn::Ptr packet);

  void connect(NetworkAddress::Ptr host, int port);
  
  Client::Ptr find_client(Link::Ptr link);
  Client::Ptr find_client(UUID id);

  typedef boost::shared_ptr<MessageSignal> MessageSignalPtr;
  
  typedef std::map<std::pair<int, int>, MessageSignalPtr> MessageSignalMap;
  typedef MessageSignalMap::iterator MessageSignalMapIter;

private:
  //! My ID
  UUID myid;

  //!
  std::string username;

  //!
  std::string secret;
  
  //! 
  Announce::Ptr announce;

  //! 
  DirectLinkManager::Ptr direct_link_manager;

  //!
  Marshaller::Ptr marshaller;
  
  //!
  Clients clients;

  //!
  MessageSignalMap message_signals;

  //!
  int cycle_failures;
};


#endif // ROUTER_HH
