// GIONetworkAddress.cc
//
// Copyright (C) 2009, 2010, 2011, 2012 Rob Caelers <robc@krandor.nl>
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
#include "GIONetworkAddress.hh"

using namespace std;

//! Creates a new connection.

GIONetworkAddress::GIONetworkAddress()
  : socket_address(NULL)
{
}

GIONetworkAddress::GIONetworkAddress(const std::string &ip, int port)
{
  GInetAddress *inet_address = g_inet_address_new_from_string(ip.c_str());
  socket_address = g_inet_socket_address_new(inet_address, port);
  g_object_unref(inet_address);
}

GIONetworkAddress::GIONetworkAddress(GSocketAddress *socket_address)
  : socket_address(socket_address)
{
  g_object_ref(socket_address);
}

GIONetworkAddress::GIONetworkAddress(GInetAddress *inet_address)
{
  socket_address = g_inet_socket_address_new(inet_address, 0);
}

GIONetworkAddress::GIONetworkAddress(const GIONetworkAddress &other)
{
  socket_address = other.socket_address;
  g_object_ref(socket_address);
}

GIONetworkAddress::~GIONetworkAddress()
{
  if (socket_address != NULL)
    {
      g_object_unref(socket_address);
    }
}

GSocketAddress *
GIONetworkAddress::address() const
{
  return socket_address;
}

GInetAddress *
GIONetworkAddress::inet_address() const
{
  return g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(socket_address));
}

int
GIONetworkAddress::port() const
{
  return g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(socket_address));
}

GSocketFamily
GIONetworkAddress::family() const
{
  return g_inet_address_get_family(inet_address());
}

bool
GIONetworkAddress::operator==(const NetworkAddress &other)
{
  bool ret = false;
  const GIONetworkAddress* otherGio = dynamic_cast<const GIONetworkAddress*>(&other);

  if (otherGio != NULL)
    {
      ret = ( g_inet_address_equal(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(otherGio->socket_address)),
                                   g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(socket_address)))  &&
              g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(otherGio->socket_address)) ==
              g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(socket_address)));
    }
  return ret;
}

GIONetworkAddress&
GIONetworkAddress::operator=(const GIONetworkAddress &other)
{
  if (this != &other)
    {
      if (socket_address != NULL)
        {
          g_object_unref(socket_address);
        }

      socket_address = other.socket_address;

      if (socket_address != NULL)
        {
          g_object_ref(socket_address);
        }
    }
  return *this;

}
#endif
