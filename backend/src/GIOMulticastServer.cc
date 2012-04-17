// GIOMulticastServer.cc
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
#include "SocketException.hh"
#include "GIOMulticastServer.hh"
#include "GIOMulticastSocket.hh"
#include "NetlinkNetworkInterfaceMonitor.hh"

using namespace std;

GIOMulticastServer::GIOMulticastServer(const std::string &multicast_ipv4, const std::string &multicast_ipv6, int multicast_port)
{
  monitor = new NetlinkNetworkInterfaceMonitor();

  GInetAddress *inet_address = g_inet_address_new_from_string(multicast_ipv4.c_str());
  multicast_address_ipv4 = g_inet_socket_address_new(inet_address, multicast_port);
  g_object_unref(inet_address);

  inet_address = g_inet_address_new_from_string(multicast_ipv6.c_str());
  multicast_address_ipv6 = g_inet_socket_address_new(inet_address, multicast_port);
  g_object_unref(inet_address);
}

GIOMulticastServer::~GIOMulticastServer()
{
  delete monitor;

  for (list<Connection *>::iterator i = connections.begin(); i != connections.end(); i++)
    {
      delete (*i)->socket;
      delete *i;
    }

  if (multicast_address_ipv4 != NULL)
    {
      g_object_unref(multicast_address_ipv4);
    }
  
  if (multicast_address_ipv6 != NULL)
    {
      g_object_unref(multicast_address_ipv6);
    }
}

void
GIOMulticastServer::init()
{
  monitor->signal_interface_changed().connect(sigc::mem_fun(*this, &GIOMulticastServer::on_interface_changed));
  monitor->init();
}

void
GIOMulticastServer::send(const gchar *buf, gsize count)
{
  for (list<Connection *>::iterator i = connections.begin(); i != connections.end(); i++)
    {
      (*i)->socket->send(buf, count);
    }
}

sigc::signal<void, int, void *> &
GIOMulticastServer::signal_multicast_data()
{
  return multicast_data_signal;
}

void
GIOMulticastServer::on_multicast_data(Connection *connection, int size, void *data)
{
  if (size == 0 && data == NULL)
    {
      // TODO: retry? create new socket?
      connections.remove(connection);
      delete connection->socket;
      delete connection;
    }
  else
    {
      multicast_data_signal.emit(size, data);
    }
}

void
GIOMulticastServer::on_interface_changed(const INetworkInterfaceMonitor::NetworkInterfaceChange &change)
{
  if (change.valid)
    {
      GSocketFamily family = g_inet_address_get_family(change.address);
      GSocketAddress *multicast_address = ( family == G_SOCKET_FAMILY_IPV4 ? multicast_address_ipv4 :
                                            family == G_SOCKET_FAMILY_IPV6 ? multicast_address_ipv6 : NULL);
                              
      GIOMulticastSocket *socket = new GIOMulticastSocket(multicast_address, change.name, change.address);
      Connection *connection = new Connection();

      g_debug("Add multicast socket %s %s", change.name.c_str(), g_inet_address_to_string(change.address));
      
      connection->adapter_name = change.name;
      connection->local_address = change.address;
      connection->socket = socket;
      
      connections.push_back(connection);
      socket->signal_multicast_data().connect(sigc::bind<0>(sigc::mem_fun(*this, &GIOMulticastServer::on_multicast_data), connection));
      socket->init();
    }
  else
    {
      for (list<Connection *>::iterator i = connections.begin(); i != connections.end(); i++)
        {
          if (change.name == (*i)->adapter_name && g_inet_address_equal(change.address, (*i)->local_address))
            {
              delete (*i)->socket;
              delete *i;
              connections.erase(i);
              break;
            }
        }
    }
}

#endif
