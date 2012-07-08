// GnetSocketDriver.cc
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
#include "GIOSocket.hh"
#include "GIONetworkAddress.hh"

using namespace std;
using namespace workrave::network;

//! Creates a new connection.
GIOSocket::GIOSocket() :
  resolver(NULL),
  socket_client(NULL),
  connection(NULL),
  socket(NULL),
  source(NULL),
  port(0),
  remote_address(NULL),
  local_address(NULL)
{
  TRACE_ENTER("GIOSocket::GIOSocket()");
  TRACE_EXIT();
}


//! Destructs the connection.
GIOSocket::~GIOSocket()
{
  TRACE_ENTER("GIOSocket::~GIOSocket");

  if (socket != NULL)
    {
      g_socket_shutdown(socket, TRUE, TRUE, NULL);
      g_socket_close(socket, NULL);
      g_object_unref(socket);
    }
      
  if (connection != NULL)
    {
      g_object_unref(connection);
    }

  if (source != NULL)
    {
      g_source_destroy(source);
    }

  if (resolver != NULL)
    {
      g_object_unref(resolver);
    }

  if (socket_client != NULL)
    {
      g_object_unref(socket_client);
    }

  if (remote_address != NULL)
    {
      g_object_unref(remote_address);
    }
  
  TRACE_EXIT();
}


void
GIOSocket::init(GSocketConnection *connection)
{
  socket = g_socket_connection_get_socket(connection);
  g_object_ref(connection);

  g_socket_set_blocking(socket, FALSE);
  g_socket_set_keepalive(socket, TRUE);

  source = g_socket_create_source(socket, (GIOCondition)G_IO_IN, NULL);
  g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
  g_source_attach(source, NULL);
  g_source_unref(source);

  remote_address = g_socket_get_remote_address(socket, NULL);
}


void
GIOSocket::connect(const string &host_name, int port)
{
  TRACE_ENTER_MSG("GIOSocket::connect", host_name << " " << port);
  this->port = port;
  
  GInetAddress *inet_addr = g_inet_address_new_from_string(host_name.c_str());
  
  if (inet_addr != NULL)
    {
      connect(g_inet_socket_address_new(inet_addr, port));
      g_object_unref(inet_addr);
    }
  else
    {
      resolver = g_resolver_get_default();
      g_resolver_lookup_by_name_async(resolver, host_name.c_str(), NULL, static_resolve_ready, this);
    }
  
  TRACE_EXIT();
}


//! Connects to the specified host.
void
GIOSocket::connect(GSocketAddress *address)
{
  TRACE_ENTER("GIOSocket::connect");
  this->remote_address = address;

  socket_client = g_socket_client_new();
      
  g_socket_client_connect_async(socket_client,
                                G_SOCKET_CONNECTABLE(remote_address),
                                NULL,
                                static_connected_callback,
                                this);

  TRACE_EXIT();
}


//! Connects to the specified host.
bool
GIOSocket::join_multicast(const GIONetworkAddress::Ptr multicast_address, const std::string &adapter, const GIONetworkAddress::Ptr local_address)
{
  TRACE_ENTER("GIOSocket::join_multicast");
  GError *error = NULL;

  this->remote_address = multicast_address->address();
  this->adapter = adapter;
  this->local_address = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(local_address->address()));
  
  GInetAddress *inet_address = multicast_address->inet_address();
  GSocketFamily family = multicast_address->family();
  
  socket = g_socket_new(family, G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &error);
  
  if (error == NULL)
    {
      g_socket_set_broadcast(socket, TRUE);
      g_socket_set_multicast_ttl(socket, 1);
      g_socket_set_multicast_loopback(socket, TRUE);
      g_socket_set_blocking(socket, FALSE);
      g_socket_set_keepalive(socket, TRUE);
      g_socket_bind(socket, remote_address, TRUE, &error);
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
      TRACE_MSG(string("Cannot create multicast socket: ") + error->message);
      g_error_free(error);
      TRACE_EXIT();
      return false;
    }
  
  TRACE_EXIT();
  return true;
}


//! Read from the connection.
bool
GIOSocket::read(gchar *buf, gsize count, gsize &bytes_read)
{
  TRACE_ENTER_MSG("GIOSocket::read", count);

  GError *error = NULL;
  gsize num_read = 0;
  bool ret = true;

  if (socket != NULL)
    {
      num_read = g_socket_receive(socket, (char *)buf, count, NULL, &error);
      if (error != NULL)
        {
          g_error_free(error);
          num_read = 0;
          ret = false;
        }
    }

  bytes_read = (int)num_read;
  TRACE_RETURN(bytes_read);
  return ret;
}



//! Read from the connection.
bool
GIOSocket::receive(gchar *buf, gsize count, gsize &bytes_read, NetworkAddress::Ptr &address)
{
  TRACE_ENTER_MSG("GIOSocket::receive", count);

  GError *error = NULL;
  gsize num_read = 0;
  GSocketAddress *src_address = NULL;
  bool ret = true;
     
  if (socket != NULL)
    {
      num_read = g_socket_receive_from(socket, &src_address, buf, count, NULL, &error);
      if (error != NULL)
        {
          g_error_free(error);
          num_read = 0;
          ret = false;
        }
      else if (src_address != NULL)
        {
          address = NetworkAddress::Ptr(new GIONetworkAddress(src_address));
          g_object_unref(src_address);
        }
    }

  bytes_read = (int)num_read;
  TRACE_RETURN(bytes_read);
  return ret;
}


//! Write to the connection.
bool
GIOSocket::write(const gchar *buf, gsize count)
{
  TRACE_ENTER_MSG("GIOSocket::write", count);

  GError *error = NULL;
  gsize num_written = 0;
  if (socket != NULL)
    {
      num_written = g_socket_send(socket, (char *)buf, count, NULL, &error);
      if (error != NULL)
        {
          g_error_free(error);
          num_written = 0;
        }
    }
  TRACE_RETURN(num_written);
  return num_written == count;
}


//! Write to the connection.
bool
GIOSocket::send(const gchar *buf, gsize count)
{
  TRACE_ENTER_MSG("GIOSocket::send", count);

  GError *error = NULL;
  gsize num_written = 0;
  if (socket != NULL)
    {
      num_written = g_socket_send_to(socket, remote_address, (char *)buf, count, NULL, &error);
      if (error != NULL)
        {
          g_error_free(error);
          num_written = 0;
        }
    }

  TRACE_RETURN(num_written);
  return num_written == count;
}



//! Close the connection.
void
GIOSocket::close()
{
  TRACE_ENTER("GIOSocket::close");
  GError *error = NULL;
  if (socket != NULL)
    {
      g_socket_shutdown(socket, TRUE, TRUE, &error);
      g_socket_close(socket, &error);
      socket = NULL;
    }
  if (error != NULL)
    {
      g_error_free(error);
    }
  TRACE_EXIT();
}


boost::signals2::signal<void()> &
GIOSocket::signal_io()
{
  return io_signal;
}

boost::signals2::signal<void()> &
GIOSocket::signal_connected()
{
  return connected_signal;
}

boost::signals2::signal<void()> &
GIOSocket::signal_disconnected()
{
  return disconnected_signal;
}

void
GIOSocket::static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
  TRACE_ENTER("GIOSocket::static_connected_callback");

  GIOSocket *self = (GIOSocket *)user_data;
  GError *error = NULL;

  GSocketConnection *socket_connection = g_socket_client_connect_finish(G_SOCKET_CLIENT(source_object), result, &error);

  if (error != NULL)
    {
      TRACE_MSG("failed to connect");
      g_error_free(error);
    }
  else if (socket_connection != NULL)
    {
      self->connection = socket_connection;
      self->socket = g_socket_connection_get_socket(self->connection);
      self->source = g_socket_create_source(self->socket, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP), NULL);

      g_source_set_callback(self->source, (GSourceFunc) static_data_callback, (void*)self, NULL);
      g_source_attach(self->source, NULL);

      g_socket_set_blocking(self->socket, FALSE);
      g_socket_set_keepalive(self->socket, TRUE);

      self->connected_signal();
    }
  TRACE_EXIT();
}


gboolean
GIOSocket::static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  TRACE_ENTER_MSG("GIOSocket::static_data_callback", (int)condition);

  GIOSocket *self = (GIOSocket *) user_data;
  gboolean ret = TRUE;

  (void) socket;

  // check for socket error
  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      self->disconnected_signal();
      ret = FALSE;
    }

  // process input
  if (ret && (condition & G_IO_IN))
    {
      self->io_signal();
    }

  TRACE_EXIT();
  return ret;
}


void
GIOSocket::static_resolve_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTER("GIOSocket::static_resolve_ready");
  bool result = false;
  GIOSocket *self = (GIOSocket *)user_data;
  GList *addresses = g_resolver_lookup_by_name_finish((GResolver *)source_object, res, NULL);

  if (addresses != NULL)
    {
      // Take first result
      if (addresses->data != NULL)
        {
          self->connect(g_inet_socket_address_new((GInetAddress *) addresses->data, self->port));
          result = true;
        }
       g_resolver_free_addresses(addresses);
    }

  if (!result)
    {
      self->disconnected_signal();
    }

  TRACE_EXIT();
}

NetworkAddress::Ptr
GIOSocket::get_remote_address()
{
  return NetworkAddress::Ptr(new GIONetworkAddress(remote_address));
}

#endif
