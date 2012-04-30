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

#include "SocketServer.hh"
#include "NetworkAddress.hh"
#include "ByteStream.hh"
#include "WRID.hh"

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

  sigc::signal<void, gsize, const gchar *, NetworkAddress::Ptr> &signal_data();
  
private:
  enum ConnectionState
    {
      CONNECTION_STATE_INVALID,
      CONNECTION_STATE_CONNECTING,
      CONNECTION_STATE_CONNECTED,
      CONNECTION_STATE_AUTHENTICATED,
      CONNECTION_STATE_CLOSING,
    };
    
  struct ConnectionInfo
  {
    typedef boost::shared_ptr<ConnectionInfo> Ptr;

    ConnectionInfo() : state(CONNECTION_STATE_INVALID) {}
    
    NetworkAddress::Ptr address;
    Socket::Ptr socket;
    boost::shared_ptr<ByteStream> stream;
    WRID id;
    ConnectionState state;
  };
    
  typedef std::list<ConnectionInfo::Ptr> Connections;
  typedef std::list<ConnectionInfo::Ptr>::iterator ConnectionIter;
  typedef std::list<ConnectionInfo::Ptr>::const_iterator ConnectionCIter;

  void on_accepted(Socket::Ptr socket);
  void on_connected(ConnectionInfo::Ptr info);
  void on_disconnected(ConnectionInfo::Ptr info);
  void on_data(ConnectionInfo::Ptr info);

  void close(ConnectionInfo::Ptr info);
  
  
private:
  //! Default server
  SocketServer::Ptr unicast_server;

  //! Connections
  Connections connections;

  //!
  sigc::signal<void, gsize, const gchar *, NetworkAddress::Ptr> data_signal;
};


#endif // NETWORKDIRECTLINK_HH
