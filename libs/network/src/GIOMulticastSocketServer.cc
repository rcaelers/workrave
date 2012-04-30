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

#include "debug.hh"
#include "GIOMulticastSocketServer.hh"
#include "GIONetworkAddress.hh"
#include "GIOSocket.hh"
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

  monitor->signal_interface_changed().connect(sigc::mem_fun(*this, &GIOMulticastSocketServer::on_interface_changed));
  return monitor->init();
  TRACE_EXIT();
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


sigc::signal<void, gsize, const gchar *, NetworkAddress::Ptr> &
GIOMulticastSocketServer::signal_data()
{
  return data_signal;
}


void
GIOMulticastSocketServer::on_data(Connection::Ptr connection)
{
  TRACE_ENTER("GIOMulticastSocketServer::on_data");
  static gchar buffer[1024];
  
  gsize bytes_read = 0;
  NetworkAddress::Ptr address;
  bool ok = connection->socket->receive(buffer, sizeof(buffer), bytes_read, address);
  if (!ok)
    {
      // TODO: check with monitor is link is still and retry.
      connection->socket->close();
      connections.remove(connection);
    }
  else
    {
      data_signal.emit(bytes_read, buffer, address);
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
                              
      GIOSocket::Ptr socket(new GIOSocket());
      Connection::Ptr connection(new Connection());

      connection->adapter_name = change.name;
      connection->local_address = change.address->inet_address();
      connection->socket = socket;
      
      connections.push_back(connection);
      socket->signal_io().connect(sigc::bind<0>(sigc::mem_fun(*this, &GIOMulticastSocketServer::on_data), connection));
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
