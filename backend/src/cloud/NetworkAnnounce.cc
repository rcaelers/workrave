// NetworkAnnounce.cc
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <fstream>

#include "debug.hh"

#include "NetworkAnnounce.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "Util.hh"

#include "networking.pb.h"

using namespace std;
using namespace workrave;
using namespace workrave::network;

//! Constructs a network announcer
NetworkAnnounce::NetworkAnnounce(const WRID &my_id) : my_id(my_id)
{
  multicast_server = MulticastSocketServer::create();
}

//! Destructs the network announcer
NetworkAnnounce::~NetworkAnnounce()
{
}

//! Initializes the network announcer.
void
NetworkAnnounce::init()
{
  TRACE_ENTER("NetworkAnnounce::init");
  multicast_server->init("239.160.181.73", "ff15::1:145", 27273);
  multicast_server->signal_data().connect(sigc::mem_fun(*this, &NetworkAnnounce::on_data));
  TRACE_EXIT();
}


//! Terminates the network announcer.
void
NetworkAnnounce::terminate()
{
  TRACE_ENTER("NetworkAnnounce::terminate");
  TRACE_EXIT();
}


//! Periodic heartbeart from the core.
void
NetworkAnnounce::heartbeat()
{
  TRACE_ENTER("NetworkAnnounce::heartbeat");

  networking::Hello hello;

  hello.set_address("foo");
  TRACE_MSG(hello.ByteSize());

  multicast_server->send(hello.SerializeAsString().c_str(), hello.ByteSize());
  TRACE_EXIT();
}


//!
void
NetworkAnnounce::on_data(gsize size, const gchar *data, NetworkAddress::Ptr na)
{
  (void) na;
  networking::Hello hello;

  hello.ParseFromArray(data, size);
  printf(">> %s", hello.address().c_str());
}
