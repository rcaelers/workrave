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
#include <boost/shared_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "network/MulticastSocketServer.hh"

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
  static Ptr create(Marshaller::Ptr marshaller);

  Announce(Marshaller::Ptr marshaller);
  virtual ~Announce();

  void init(int port);
  void terminate();

  void send_message(const std::string &message);

  typedef boost::signals2::signal<bool(Link::Ptr, PacketIn::Ptr)> data_signal_type;
  
  data_signal_type &signal_data();
 
private:
  void on_data(gsize size, const gchar *data, NetworkAddress::Ptr na);
  
private:
  //!
  Marshaller::Ptr marshaller;
  
  //! Default server
  MulticastSocketServer::Ptr multicast_server;

  //!
  data_signal_type data_signal;
};


#endif // ANNOUNCE_HH
