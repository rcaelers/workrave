// DirectLinkManager.hh --- ing network server
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

#ifndef DIRECTLINKMANAGER_HH
#define DIRECTLINKMANAGER_HH

#include "DirectLink.hh"

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "network/SocketServer.hh"
#include "network/NetworkAddress.hh"

#include "ByteStream.hh"
#include "Marshaller.hh"

using namespace workrave;
using namespace workrave::network;

class DirectLinkManager
{
public:
  typedef boost::shared_ptr<DirectLinkManager> Ptr;

public:
  static Ptr create(Marshaller::Ptr marshaller);

  DirectLinkManager(Marshaller::Ptr marshaller);
  virtual ~DirectLinkManager();

  void init(int port);
  void terminate();

  boost::signals2::signal<void(DirectLink::Ptr)> &signal_new_link();
  
private:
  void on_accepted(Socket::Ptr socket);
 
private:
  Marshaller::Ptr marshaller;
  
  //! Default server
  SocketServer::Ptr unicast_server;

  //!
  boost::signals2::signal<void(DirectLink::Ptr)> new_link_signal;
};


#endif // DIRECTLINKMANAGER_HH
