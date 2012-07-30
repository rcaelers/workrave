// Announce.cc
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

#include <string>
#include <boost/shared_ptr.hpp>

#include "debug.hh"

#include "Announce.hh"

using namespace std;

Announce::Ptr
Announce::create(Marshaller::Ptr marshaller)
{
  return Announce::Ptr(new Announce(marshaller));
}


Announce::Announce(Marshaller::Ptr marshaller) : marshaller(marshaller)
{
  TRACE_ENTER("Announce::Announce");
  multicast_server = MulticastSocketServer::create();
  TRACE_EXIT();
}


Announce::~Announce()
{
  TRACE_ENTER("Announce::~Announce");
  TRACE_EXIT();
}


void
Announce::init(int port)
{
  TRACE_ENTER("Announce::init");
  multicast_server->init("239.160.181.73", "ff15::1:145", port);
  multicast_server->signal_data().connect(boost::bind(&Announce::on_data, this, _1, _2, _3));
  TRACE_EXIT();
}


void
Announce::terminate()
{
  TRACE_ENTER("Announce::terminate");
  TRACE_EXIT();
}


void
Announce::send_message(const std::string &message)
{
  TRACE_ENTER("Announce::send_message");
  multicast_server->send(message.c_str(), message.length());
  TRACE_EXIT();
}

//!
void
Announce::on_data(gsize size, const gchar *data, NetworkAddress::Ptr na)
{
  TRACE_ENTER("Router::on_multicast_data");

  EphemeralLink::Ptr link = EphemeralLink::create();
  link->state = Link::CONNECTION_STATE_CONNECTED;
  link->address = na;

  PacketIn::Ptr packet = marshaller->unmarshall(size, data);
  if (packet)
    {
      data_signal(link, packet);
    }
  
  TRACE_EXIT();
}

Announce::data_signal_type &
Announce::signal_data()
{
  return data_signal;
}
