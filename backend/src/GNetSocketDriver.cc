// GnetSocketDriver.cc
//
// Copyright (C) 2002, 2003, 2004, 2005, 2007, 2008, 2010 Rob Caelers <robc@krandor.nl>
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

#if defined(HAVE_DISTRIBUTION) && defined(HAVE_GNET)

#define GNET_EXPERIMENTAL

#include "debug.hh"
#include "GNetSocketDriver.hh"

using namespace std;


//! Creates a new listen socket.
GNetSocketServer::GNetSocketServer() :
  socket(NULL),
  iochannel(NULL),
  watch_flags(0),
  watch(0)
{
}


//! Destructs the listen socket.
GNetSocketServer::~GNetSocketServer()
{
#ifndef HAVE_GNET2
  if (iochannel != NULL)
    {
      // this causes troubles with gnet2
      g_io_channel_unref(iochannel);
    }
#endif

  if (socket != NULL)
    {
      gnet_tcp_socket_delete(socket);
    }

  if (watch != 0)
    {
      g_source_remove(watch);
    }
}


//! Listen at the specified port.
void
GNetSocketServer::listen(int port)
{
  // set_listener MUST have been called.
  g_assert(listener != NULL);

  socket = gnet_tcp_socket_server_new_with_port(port);

  if (socket == NULL)
    {
      throw SocketException("listen error");
    }

  gnet_tcp_socket_server_accept_async(socket, static_async_accept, this);
}


//! GNet has accepted a new connection.
void
GNetSocketServer::static_async_accept(GTcpSocket *server, GTcpSocket *client, gpointer data)
{
  GNetSocketServer *s = (GNetSocketServer *)data;
  s->async_accept(server, client);
}


//! GNet has accepted a new connection.
void
GNetSocketServer::async_accept(GTcpSocket *server, GTcpSocket *client)
{
  (void) server;

  try
    {
      GNetSocket *socket =  new GNetSocket(client);
      listener->socket_accepted(this, socket);
    }
  catch(...)
    {
      // Make sure that no exception reach the glib mainloop.
    }
}


//! GNet reports that data is ready to be read.
gboolean
GNetSocket::static_async_io(GIOChannel *iochannel, GIOCondition condition,
                            gpointer data)
{
  GNetSocket *con =  (GNetSocket *)data;
  return con->async_io(iochannel, condition);
}


//! GNet reports that data is ready to be read.
bool
GNetSocket::async_io(GIOChannel *iochannel, GIOCondition condition)
{
  (void) iochannel;

  bool ret = true;

  try
    {
      // check for socket error
      if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
        {
          if (listener != NULL)
            {
              close();
              listener->socket_closed(this, user_data);
            }
          ret = false;
        }

      // process input
      if (ret && (condition & G_IO_IN))
        {
          if (listener != NULL)
            {
              listener->socket_io(this, user_data);
            }
        }
    }
  catch(...)
    {
      // Make sure that no exception reach the glib mainloop.
      close();
      ret = false;
    }

  return ret;
}


//! GNet reports that the connection is established.
void
GNetSocket::async_connected(GTcpSocket *socket, GInetAddr *ia,
                            GTcpSocketConnectAsyncStatus status)
{
  try
    {
      if (status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK)
        {
          gnet_tcp_socket_delete(socket);
          socket = NULL;

          if (listener != NULL)
            {
              listener->socket_closed(this, user_data);
            }
        }
      else
        {
          socket = socket;
          iochannel = gnet_tcp_socket_get_io_channel(socket);
          watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
          watch = g_io_add_watch(iochannel, (GIOCondition)watch_flags, static_async_io, this);

          if (listener != NULL)
            {
              listener->socket_connected(this, user_data);
            }

          gnet_inetaddr_delete(ia);
        }
    }
  catch (...)
    {
      // Make sure that no exception reach the glib mainloop.
    }
}


//! Connection established.
void
GNetSocket::static_async_connected(GTcpSocket *socket,
                                   GTcpSocketConnectAsyncStatus status,
                                   gpointer data)
{
  GNetSocket *con =  (GNetSocket *)data;
  GInetAddr *ia = NULL;
  if (socket != NULL)
    {
      ia = gnet_tcp_socket_get_remote_inetaddr(socket);
    }

  con->async_connected(socket, ia, status);
}


//! Creates a new connection.
GNetSocket::GNetSocket(GTcpSocket *socket) :
  socket(socket)
{
  iochannel = gnet_tcp_socket_get_io_channel(socket);
  watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
  watch = g_io_add_watch(iochannel, (GIOCondition) watch_flags, static_async_io, this);
}


//! Creates a new connection.
GNetSocket::GNetSocket() :
  socket(NULL),
  iochannel(NULL),
  watch_flags(0),
  watch(0)
{
}


//! Destructs the connection.
GNetSocket::~GNetSocket()
{

  if (socket != NULL)
    {
      gnet_tcp_socket_delete(socket);
    }

  if (watch != 0)
    {
      g_source_remove(watch);
    }
}


//! Connects to the specified host.
void
GNetSocket::connect(const string &host, int port)
{
  gnet_tcp_socket_connect_async(host.c_str(), port, static_async_connected, this);
}


//! Read from the connection.
void
GNetSocket::read(void *buf, int count, int &bytes_read)
{
  if (iochannel == NULL)
    {
      throw SocketException("socket not connected");
    }

  gsize num_read = 0;
  GIOError error = g_io_channel_read(iochannel, (char *)buf, (gsize)count, &num_read);

  if (error != G_IO_ERROR_NONE)
    {
      throw SocketException("read error");
    }

  bytes_read = (int)num_read;
}


//! Write to the connection.
void
GNetSocket::write(void *buf, int count, int &bytes_written)
{
  if (iochannel == NULL)
    {
      throw SocketException("socket not connected");
    }

  gsize num_written = 0;
  GIOError error = g_io_channel_write(iochannel, (char *)buf, (gsize)count, &num_written);

  if (error != G_IO_ERROR_NONE)
    {
      throw SocketException("write error");
    }

  bytes_written = (int) num_written;
}


//! Close the connection.
void
GNetSocket::close()
{
#ifndef HAVE_GNET2
  if (iochannel != NULL)
    {
      // this causes troubles with gnet2
      g_io_channel_unref(iochannel);
      iochannel = NULL;
    }
#endif

  if (socket != NULL)
    {
      gnet_tcp_socket_delete(socket);
      socket = NULL;
    }

  if (watch != 0)
    {
      g_source_remove(watch);
    }

  watch = 0;
  watch_flags = 0;
}

//! Create a new socket
ISocket *
GNetSocketDriver::create_socket()
{
  return new GNetSocket();
}


//! Create a new listen socket
ISocketServer *
GNetSocketDriver::create_server()
{
  return new GNetSocketServer();
}

#endif

