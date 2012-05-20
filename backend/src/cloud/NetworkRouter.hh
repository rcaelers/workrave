// NetworkRouter.hh --- Networking network server
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

#ifndef NETWORKROUTER_HH
#define NETWORKROUTER_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <google/protobuf/message.h>

#include "INetwork.hh"
#include "NetworkAnnounce.hh"
#include "NetworkDirectLink.hh"

#include "workrave.pb.h"

using namespace workrave;
using namespace workrave::network;

class NetworkRouter : public INetwork
{
public:
  typedef boost::shared_ptr<NetworkRouter> Ptr;
  
public:
  static Ptr create();
  
public:  
  NetworkRouter();
  virtual ~NetworkRouter();

  void init(int port, std::string username, std::string secret);
  void terminate();

  virtual void connect(const std::string &host, int port);
  virtual void send_message(NetworkMessageBase::Ptr msg);

  MessageSignal &signal_message(int domain, int id);

private:
  void process_message(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message);

  const std::string marshall_message(NetworkMessageBase::Ptr message);
  const std::string marshall_message(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message);
  bool unmarshall_message(gsize size, const gchar *data, NetworkClient::Ptr client,
                          boost::shared_ptr<google::protobuf::Message> &result_message,
                          boost::shared_ptr<workrave::Header> &result_header);

  std::string get_namespace_of_domain(int domain);
  void init_myid(int instanceid);
  const std::string get_nonce() const;
  bool check_message_authentication(boost::shared_ptr<workrave::Header> header, NetworkClient::Ptr client);
  void add_message_authentication(boost::shared_ptr<workrave::Header> header);

  void on_data(gsize size, const gchar *data, NetworkClient::Ptr client);
  void on_client_changed(NetworkClient::Ptr client);

  void fire_message_signal(int domain, int id, NetworkMessageBase::Ptr);

  void send_message_except(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message, NetworkClient::Ptr client);
  
  typedef std::list<NetworkClient::Ptr> Clients;
  typedef std::list<NetworkClient::Ptr>::iterator ClientIter;
  typedef std::list<NetworkClient::Ptr>::const_iterator ClientCIter;

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
  NetworkAnnounce::Ptr announce;

  //! 
  NetworkDirectLink::Ptr direct_links;

  //!
  Clients clients;

  //!
  MessageSignalMap message_signals;
};


#endif // NETWORKROUTER_HH
