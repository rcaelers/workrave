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

#include <google/protobuf/message.h>

#include "Cloud.hh"

#include "Announce.hh"
#include "DirectLinkManager.hh"
#include "DirectLink.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::cloud;

class Router : public ICloud
{
public:
  typedef boost::shared_ptr<Router> Ptr;
  
public:
  static Ptr create();

public:  
  Router();
  virtual ~Router();

  virtual void init(int port, std::string username, std::string secret);
  virtual void terminate();
  virtual void connect(const std::string &host, int port);
  virtual void send_message(Message::Ptr message, MessageParams::Ptr params);
  virtual MessageSignal &signal_message(int domain, int id);

private:
  void init_myid(int instanceid);

  bool on_data(PacketIn::Ptr packet, Link::Ptr link, Scope scope);
  void on_direct_link_created(DirectLink::Ptr link);
  void on_direct_link_state_changed(DirectLink::Ptr link);

  void fire_message_signal(int domain, int id, Message::Ptr, MessageContext::Ptr);

  void forward_message(PacketIn::Ptr packet);
  
  void send_alive();
  void process_alive(Link::Ptr link, PacketIn::Ptr packet);
  
  typedef std::list<Link::Ptr> Links;
  typedef std::list<Link::Ptr>::iterator LinkIter;
  typedef std::list<Link::Ptr>::const_iterator LinkCIter;

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
  Links links;

  //!
  MessageSignalMap message_signals;
};


#endif // ROUTER_HH
