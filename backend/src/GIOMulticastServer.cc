// GIOMulticastSocket.cc
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
#include "GIOMulticastServer.hh"
#include "SocketException.hh"

using namespace std;

#include "NetlinkNetworkInterfaceMonitor.hh"

GIOMulticastServer::GIOMulticastServer(const std::string &multicast_ipv4, const std::string &multicast_ipv6, int multicast_port)
{
  monitor = new NetlinkNetworkInterfaceMonitor();
  monitor->signal_interface_changed().connect(sigc::mem_fun(*this, &GIOMulticastServer::on_interface_changed));

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
    }
}

void
GIOMulticastServer::init()
{
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

void
GIOMulticastServer::on_multicast_data(Connection *connection, int size, void *data)
{
  if (size == 0 && data == NULL)
    {
      delete connection->socket;
      connections.remove(connection);
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
              connections.erase(i);
              break;
            }
        }
    }
}


gboolean
GIOMulticastSocket::static_data_callback(GSocket *socket,
                                         GIOCondition condition,
                                         gpointer user_data)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::static_data_callback", (int)condition);

  GIOMulticastSocket *giosocket = (GIOMulticastSocket *)user_data;
  gboolean ret = TRUE;

  (void) socket;

  // check for socket error
  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      TRACE_MSG("Closing socket");
      giosocket->multicast_data_signal.emit(0, NULL);
      ret = FALSE;
    }
      
  // process input
  if (ret && (condition & G_IO_IN))
    {
      char buffer[1024];

      GSocketAddress *src_address = NULL;
      GError *error = NULL;
      gsize num_read = 0;

      num_read = g_socket_receive_from(giosocket->socket, &src_address, buffer, sizeof(buffer), NULL, &error);
      g_object_unref(src_address);

      if (error != NULL)
        {
          TRACE_MSG(string("socket read error: ") + error->message);
          g_error_free(error);
        }
      else
        {
          printf("%s", buffer);
          giosocket->multicast_data_signal.emit(num_read, buffer);
        }
    }

  TRACE_EXIT();
  return ret;
}


//! Creates a new connection.
GIOMulticastSocket::GIOMulticastSocket(GSocketAddress *multicast_address, const std::string &adapter, GInetAddress *local_address) :
  adapter(adapter),
  local_address(local_address),
  multicast_address(multicast_address),  
  socket(NULL),
  source(NULL)
{
  TRACE_ENTER("GIOMulticastSocket::GIOMulticastSocket()");

  TRACE_EXIT();
}


//! Destructs the connection.
GIOMulticastSocket::~GIOMulticastSocket()
{
  TRACE_ENTER("GIOMulticastSocket::~GIOMulticastSocket");
  if (socket != NULL)
    {
      g_object_unref(socket);
    }

  if (source != NULL)
    {
      g_source_destroy(source);
    }
  TRACE_EXIT();
}


//! Connects to the specified host.
bool
GIOMulticastSocket::init()
{
  TRACE_ENTER("GIOMulticastSocket::init");
  GError *error = NULL;

  GInetAddress *inet_address = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(multicast_address));
  GSocketFamily family = g_inet_address_get_family(inet_address);
  
  socket = g_socket_new(family, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
  
  if (error == NULL)
    {
      g_socket_set_broadcast(socket, TRUE);
      g_socket_set_multicast_ttl(socket, 1);
      g_socket_set_multicast_loopback(socket, TRUE);
      g_socket_set_blocking(socket, FALSE);
      g_socket_set_keepalive(socket, TRUE);
      g_socket_bind(socket, multicast_address, TRUE, &error);
    }

  if (error == NULL)
    {
      g_socket_join_multicast_group(socket, inet_address, FALSE, adapter.c_str(), &error);
    }

  if (error == NULL)
    {
      source = g_socket_create_source(socket, (GIOCondition)(G_IO_IN | G_IO_ERR), NULL);
      g_source_set_callback(source, (GSourceFunc) static_data_callback, (void *) this, NULL);
      g_source_attach(source, g_main_context_get_thread_default());
      g_source_unref(source);
    }

  if (error != NULL)
    {
      g_debug("Cannot create multicast socket: %s", error->message);
      TRACE_MSG(string("Cannot create multicast socket: ") + error->message);
      g_error_free(error);
      TRACE_EXIT();
      return false;
    }
  
  TRACE_EXIT();
  return true;
}


//! Write to the connection.
bool
GIOMulticastSocket::send(const gchar *buf, gsize count)
{
  GError *error = NULL;
  gsize num_written = 0;
  if (socket != NULL)
    {
      num_written = g_socket_send_to(socket, multicast_address, (char *)buf, count, NULL, &error);
      if (error != NULL)
        {
          g_error_free(error);
          num_written = 0;
        }
    }

  return num_written == count;
}


//! Close the connection.
void
GIOMulticastSocket::close()
{
  TRACE_ENTER("GIOMulticastSocket::close");
  GError *error = NULL;
  if (socket != NULL)
    {
      g_socket_shutdown(socket, TRUE, TRUE, &error);
      g_socket_close(socket, &error);
      socket = NULL;
    }
  TRACE_EXIT();
}

sigc::signal<void, int, void *> &
GIOMulticastSocket::signal_multicast_data()
{
  return multicast_data_signal;
}

#endif
