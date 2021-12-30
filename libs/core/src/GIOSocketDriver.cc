// Copyright (C) 2009, 2010, 2011 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#if defined(HAVE_GIO_NET) && defined(HAVE_DISTRIBUTION)

#  include "debug.hh"
#  include "GIOSocketDriver.hh"

using namespace std;

//! Destructs the listen socket.
GIOSocketServer::~GIOSocketServer()
{
  if (service != nullptr)
    {
      g_socket_service_stop(service);
      g_object_unref(service);
      service = nullptr;
    }
}

//! Listen at the specified port.
void
GIOSocketServer::listen(int port)
{
  GError *error = nullptr;

  service = g_socket_service_new();
  if (service == nullptr)
    {
      throw SocketException("Failed to create server");
    }

  gboolean rc = g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), port, nullptr, &error);
  if (!rc)
    {
      g_object_unref(service);
      service = nullptr;

      throw SocketException(string("Failed to listen: ") + error->message);
    }

  g_signal_connect(service, "incoming", G_CALLBACK(static_socket_incoming), this);
  g_socket_service_start(service);
}

gboolean
GIOSocketServer::static_socket_incoming(GSocketService *service, GSocketConnection *connection, GObject *src_object, gpointer user_data)
{
  TRACE_ENTRY();
  (void)service;
  (void)src_object;

  try
    {
      GIOSocketServer *ss = (GIOSocketServer *)user_data;
      GIOSocket *socket = new GIOSocket(connection);
      ss->listener->socket_accepted(ss, socket);
    }
  catch (...)
    {
      // Make sure that no exception reach the glib mainloop.
    }

  return FALSE;
}

void
GIOSocket::static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
  TRACE_ENTRY();
  GIOSocket *socket = (GIOSocket *)user_data;
  GError *error = nullptr;

  GSocketConnection *socket_connection = g_socket_client_connect_finish(G_SOCKET_CLIENT(source_object), result, &error);

  if (error != nullptr)
    {
      TRACE_MSG("failed to connect");
      g_error_free(error);
    }
  else if (socket_connection != nullptr)
    {
      socket->connection = socket_connection;
      socket->socket = g_socket_connection_get_socket(socket->connection);
      g_socket_set_blocking(socket->socket, FALSE);
      g_socket_set_keepalive(socket->socket, TRUE);

      socket->source = g_socket_create_source(socket->socket, (GIOCondition)(G_IO_IN | G_IO_ERR | G_IO_HUP), nullptr);
      g_source_set_callback(socket->source, reinterpret_cast<GSourceFunc>(static_data_callback), (void *)socket, nullptr);
      g_source_attach(socket->source, nullptr);
      // g_source_unref(source);

      if (socket->listener != nullptr)
        {
          socket->listener->socket_connected(socket, socket->user_data);
        }
    }
}

gboolean
GIOSocket::static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  TRACE_ENTRY_PAR((int)condition);

  GIOSocket *giosocket = (GIOSocket *)user_data;
  gboolean ret = TRUE;

  (void)socket;

  try
    {
      // check for socket error
      if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
        {
          if (giosocket->listener != nullptr)
            {
              TRACE_MSG("Closing socket");
              // giosocket->close();
              giosocket->listener->socket_closed(giosocket, giosocket->user_data);
            }
          ret = FALSE;
        }

      // process input
      if (ret && (condition & G_IO_IN))
        {
          if (giosocket->listener != nullptr)
            {
              giosocket->listener->socket_io(giosocket, giosocket->user_data);
            }
        }
    }
  catch (...)
    {
      // Make sure that no exception reach the glib mainloop.
      TRACE_MSG("Exception. Closing socket");
      giosocket->close();
      ret = FALSE;
    }
  return ret;
}

//! Creates a new connection.
GIOSocket::GIOSocket(GSocketConnection *connection)
  : connection(connection)
  , resolver(nullptr)
{
  TRACE_ENTRY();
  socket = g_socket_connection_get_socket(connection);
  g_object_ref(connection);

  g_socket_set_blocking(socket, FALSE);
  g_socket_set_keepalive(socket, TRUE);

  source = g_socket_create_source(socket, (GIOCondition)G_IO_IN, nullptr);
  g_source_set_callback(source, reinterpret_cast<GSourceFunc>(static_data_callback), (void *)this, nullptr);
  g_source_attach(source, nullptr);
  g_source_unref(source);
}

//! Creates a new connection.
GIOSocket::GIOSocket()

{
  TRACE_ENTRY();
}

//! Destructs the connection.
GIOSocket::~GIOSocket()
{
  TRACE_ENTRY();
  if (connection != nullptr)
    {
      g_object_unref(connection);
    }
  if (resolver != nullptr)
    {
      g_object_unref(resolver);
    }
  if (source != nullptr)
    {
      g_source_destroy(source);
    }
}

//! Connects to the specified host.
void
GIOSocket::connect(const string &host, int port)
{
  TRACE_ENTRY_PAR(host, port);
  this->port = port;

  GInetAddress *inet_addr = g_inet_address_new_from_string(host.c_str());
  if (inet_addr != nullptr)
    {
      connect(inet_addr, port);
      g_object_unref(inet_addr);
    }
  else
    {
      resolver = g_resolver_get_default();
      g_resolver_lookup_by_name_async(resolver, host.c_str(), nullptr, static_connect_after_resolve, this);
    }
}

void
GIOSocket::connect(GInetAddress *inet_addr, int port)
{
  TRACE_ENTRY_PAR(port);
  GSocketAddress *socket_address = g_inet_socket_address_new(inet_addr, port);
  GSocketClient *socket_client = g_socket_client_new();

  g_socket_client_connect_async(socket_client, G_SOCKET_CONNECTABLE(socket_address), nullptr, static_connected_callback, this);
}

void
GIOSocket::static_connect_after_resolve(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTRY();
  GError *error = nullptr;
  GList *addresses = g_resolver_lookup_by_name_finish((GResolver *)source_object, res, &error);

  if (error != nullptr)
    {
      TRACE_MSG("failed");
      g_error_free(error);
    }

  if (addresses != nullptr)
    {
      // Take first result
      if (addresses->data != nullptr)
        {
          GInetAddress *a = (GInetAddress *)addresses->data;
          GIOSocket *socket = (GIOSocket *)user_data;
          socket->connect(a, socket->port);
        }
      g_resolver_free_addresses(addresses);
    }
}

//! Read from the connection.
void
GIOSocket::read(void *buf, int count, int &bytes_read)
{
  TRACE_ENTRY_PAR(count);

  GError *error = nullptr;
  gsize num_read = 0;

  if (socket != nullptr)
    {
      num_read = g_socket_receive(socket, (char *)buf, count, nullptr, &error);

      if (error != nullptr)
        {
          throw SocketException(string("socket read error: ") + error->message);
        }
    }

  bytes_read = (int)num_read;
  TRACE_VAR(bytes_read);
}

//! Write to the connection.
void
GIOSocket::write(void *buf, int count, int &bytes_written)
{
  GError *error = nullptr;
  gsize num_written = 0;
  if (socket != nullptr)
    {
      num_written = g_socket_send(socket, (char *)buf, count, nullptr, &error);
      if (error != nullptr)
        {
          throw SocketException(string("socket write error: ") + error->message);
        }
    }
  bytes_written = (int)num_written;
}

//! Close the connection.
void
GIOSocket::close()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  if (socket != nullptr)
    {
      g_socket_shutdown(socket, TRUE, TRUE, &error);
      g_socket_close(socket, &error);
      socket = nullptr;
    }
}

//! Create a new socket
ISocket *
GIOSocketDriver::create_socket()
{
  return new GIOSocket();
}

//! Create a new listen socket
ISocketServer *
GIOSocketDriver::create_server()
{
  return new GIOSocketServer();
}

#endif
