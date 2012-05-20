// NetworkDirectLink.hh --- Networking network server
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

#ifndef NETWORKDIRECTLINK_HH
#define NETWORKDIRECTLINK_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "NetworkClient.hh"

#include "SocketServer.hh"
#include "NetworkAddress.hh"
#include "ByteStream.hh"

using namespace workrave;
using namespace workrave::network;

class NetworkDirectLink
{
public:
  typedef boost::shared_ptr<NetworkDirectLink> Ptr;

public:
  static Ptr create();

  NetworkDirectLink();
  virtual ~NetworkDirectLink();

  void init(int port);
  void terminate();

  void connect(const std::string &host, int port);
  void send_message(const std::string &message);
  void send_message_to(const std::string &message, NetworkClient::Ptr client);
  void send_message_except(const std::string &message, NetworkClient::Ptr client);

  boost::signals2::signal<void(gsize, const gchar *, NetworkClient::Ptr)> &signal_data();
  boost::signals2::signal<void(NetworkClient::Ptr)> &signal_client_update();
  
private:
  class NetworkDirectLinkClient : public NetworkClient
    {
    public:
      typedef boost::shared_ptr<NetworkDirectLinkClient> Ptr;

      static Ptr create(Scope scope);

      NetworkDirectLinkClient(Scope scope) : NetworkClient(scope)
      {
        stream = boost::shared_ptr<ByteStream>(new ByteStream(1024));
      }
    
      Socket::Ptr socket;
      boost::shared_ptr<ByteStream> stream;
  };
    
  typedef std::list<NetworkDirectLinkClient::Ptr> Connections;
  typedef std::list<NetworkDirectLinkClient::Ptr>::iterator ConnectionIter;
  typedef std::list<NetworkDirectLinkClient::Ptr>::const_iterator ConnectionCIter;

  void on_accepted(Socket::Ptr socket);
  void on_connected(NetworkDirectLinkClient::Ptr info);
  void on_disconnected(NetworkDirectLinkClient::Ptr info);
  void on_data(NetworkDirectLinkClient::Ptr info);

  void close(NetworkDirectLinkClient::Ptr info);
  NetworkDirectLinkClient::Ptr create_info_for_socket(Socket::Ptr socket, NetworkClient::State state);
  
private:
  //! Default server
  SocketServer::Ptr unicast_server;

  //! Connections
  Connections connections;

  //!
  boost::signals2::signal<void(gsize, const gchar *, NetworkClient::Ptr)> data_signal;

  //!
  boost::signals2::signal<void(NetworkClient::Ptr)> client_update_signal;
};


#endif // NETWORKDIRECTLINK_HH
