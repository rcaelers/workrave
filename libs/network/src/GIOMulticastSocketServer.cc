// GIOMulticastSocketServer.cc
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_GIO_NET)

#include "GIOMulticastSocketServer.hh"

#include "debug.hh"
#include "GIONetworkAddress.hh"
#include "GIOMulticastSocket.hh"
#include "NetworkInterfaceMonitor.hh"

using namespace std;
using namespace workrave::network;

GIOMulticastSocketServer::GIOMulticastSocketServer()
{
  monitor = NetworkInterfaceMonitor::create();
}


GIOMulticastSocketServer::~GIOMulticastSocketServer()
{
}


bool
GIOMulticastSocketServer::init(const std::string &address_ipv4, const std::string &address_ipv6, int port)
{
  TRACE_ENTER("GIOMulticastSocketServer::init");

  this->address_ipv4 = GIONetworkAddress::Ptr(new GIONetworkAddress(address_ipv4, port));
  this->address_ipv6 = GIONetworkAddress::Ptr(new GIONetworkAddress(address_ipv6, port));

  monitor->signal_interface_changed().connect(boost::bind(&GIOMulticastSocketServer::on_interface_changed, this, _1));
  bool ret = monitor->init();
  TRACE_EXIT();
  return ret;
}


void
GIOMulticastSocketServer::send(const gchar *buf, gsize count)
{
  TRACE_ENTER("GIOMulticastSocketServer::send");
  for (list<Connection::Ptr>::iterator i = connections.begin(); i != connections.end(); i++)
    {
      (*i)->socket->send(buf, count);
    }
  TRACE_EXIT();
}


boost::signals2::signal<void(gsize, const gchar *, NetworkAddress::Ptr)> &
GIOMulticastSocketServer::signal_data()
{
  return data_signal;
}


void
GIOMulticastSocketServer::on_data(Connection::Ptr connection)
{
  TRACE_ENTER("GIOMulticastSocketServer::on_data");
  gchar buffer[1024]; // FIXME: 
  
  gsize bytes_read = 0;
  NetworkAddress::Ptr address;
  bool ok = connection->socket->receive(buffer, sizeof(buffer), bytes_read, address);
  if (!ok)
    {
      // TODO: check with monitor if link is still up and retry.
      connection->socket->close();
      connections.remove(connection);
    }
  else
    {
      data_signal(bytes_read, buffer, address);
    }
  TRACE_EXIT();
}


void
GIOMulticastSocketServer::on_interface_changed(const NetworkInterfaceMonitor::NetworkInterfaceInfo &change)
{
  TRACE_ENTER_MSG("GIOMulticastSocketServer::on_interface_changed", change.name << " " << change.valid);
  if (change.valid)
    {
      GSocketFamily family = change.address->family();
      GIONetworkAddress::Ptr multicast_address ( family == G_SOCKET_FAMILY_IPV4 ? address_ipv4 :
                                                 family == G_SOCKET_FAMILY_IPV6 ? address_ipv6 : address_ipv6);
                              
      GIOMulticastSocket::Ptr socket(new GIOMulticastSocket());
      Connection::Ptr connection(new Connection());

      connection->adapter_name = change.name;
      connection->local_address = change.address->inet_address();
      connection->socket = socket;
      
      connections.push_back(connection);
      socket->signal_io().connect(boost::bind(&GIOMulticastSocketServer::on_data, this, connection));
      socket->join_multicast(multicast_address, change.name, change.address);
    }
  else
    {
      for (list<Connection::Ptr>::iterator i = connections.begin(); i != connections.end(); i++)
        {
          if (change.name == (*i)->adapter_name && g_inet_address_equal(change.address->inet_address(), (*i)->local_address))
            {
              (*i)->socket->close();
              connections.erase(i);
              break;
            }
        }
    }
  TRACE_EXIT();
}

#endif
