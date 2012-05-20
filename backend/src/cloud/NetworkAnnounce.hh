// NetworkAnnounce.hh --- Networking network server
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

#ifndef NETWORKANNOUNCE_HH
#define NETWORKANNOUNCE_HH

#include <string>
#include <boost/shared_ptr.hpp>

#include "MulticastSocketServer.hh"
#include "NetworkClient.hh"

using namespace workrave;
using namespace workrave::network;

class NetworkAnnounce
{
public:
  typedef boost::shared_ptr<NetworkAnnounce> Ptr;

public:
  static Ptr create();

  NetworkAnnounce();
  virtual ~NetworkAnnounce();

  void init(int port);
  void terminate();

  void send_message(const std::string &message);
 
  boost::signals2::signal<void(gsize, const gchar *, NetworkClient::Ptr)> &signal_data();
 
private:
  void on_data(gsize size, const gchar *data, NetworkAddress::Ptr na);
  
private:
  //! Default server
  MulticastSocketServer::Ptr multicast_server;

  //!
  boost::signals2::signal<void(gsize, const gchar *, NetworkClient::Ptr)> data_signal;
};


#endif // NETWORKANNOUNCE_HH
