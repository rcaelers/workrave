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

#include <string>
#include <boost/shared_ptr.hpp>

#include "debug.hh"

#include "NetworkAnnounce.hh"

using namespace std;

NetworkAnnounce::Ptr
NetworkAnnounce::create()
{
  return NetworkAnnounce::Ptr(new NetworkAnnounce());
}


NetworkAnnounce::NetworkAnnounce()
{
  TRACE_ENTER("NetworkAnnounce::NetworkAnnounce");
  multicast_server = MulticastSocketServer::create();
  TRACE_EXIT();
}


NetworkAnnounce::~NetworkAnnounce()
{
  TRACE_ENTER("NetworkAnnounce::~NetworkAnnounce");
  TRACE_EXIT();
}


void
NetworkAnnounce::init(int port)
{
  TRACE_ENTER("NetworkAnnounce::init");
  multicast_server->init("239.160.181.73", "ff15::1:145", port);
  multicast_server->signal_data().connect(boost::bind(&NetworkAnnounce::on_data, this, _1, _2, _3));
  TRACE_EXIT();
}


void
NetworkAnnounce::terminate()
{
  TRACE_ENTER("NetworkAnnounce::terminate");
  TRACE_EXIT();
}


void
NetworkAnnounce::send_message(const std::string &message)
{
  TRACE_ENTER("NetworkAnnounce::send_message");
  multicast_server->send(message.c_str(), message.length());
  TRACE_EXIT();
}

//!
void
NetworkAnnounce::on_data(gsize size, const gchar *data, NetworkAddress::Ptr na)
{
  TRACE_ENTER("NetworkRouter::on_multicast_data");

  NetworkClient::Ptr info = NetworkClient::create(NetworkClient::SCOPE_MULTICAST);
  info->state = NetworkClient::CONNECTION_STATE_CONNECTED;
  info->address = na;

  data_signal(size, data, info);
  TRACE_EXIT();
}

boost::signals2::signal<void(gsize, const gchar *, NetworkClient::Ptr)> &
NetworkAnnounce::signal_data()
{
  return data_signal;
}
