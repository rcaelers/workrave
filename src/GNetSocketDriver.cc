// GnetSocketDriver.cc
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//

static const char rcsid[] = "$Id$";

#define GNET_EXPERIMENTAL

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <assert.h>

#include "GNetSocketDriver.hh"


//! Constructs a new Gnet socket driver.
GNetSocketDriver::GNetSocketDriver()
{
}


//! Destructs the socket driver.
GNetSocketDriver::~GNetSocketDriver()
{
}


//! Initializes the driver.
bool
GNetSocketDriver::init()
{
  return true;
}


//! Returns the canonicalize hostname of the remote host.
char *
GNetSocketDriver::get_my_canonical_name()
{
#ifdef HAVE_GNET2
  GInetAddr *ia = gnet_inetaddr_get_host_addr();
#else
  GInetAddr *ia = gnet_inetaddr_gethostaddr();
#endif
  
  g_assert(ia != NULL);
  char *name = gnet_inetaddr_get_canonical_name(ia);
  gnet_inetaddr_delete(ia);

  return name;
}


//! Returns the canonical hostname of the specified hostname.
char *
GNetSocketDriver::canonicalize(char *host)
{
  char *ret = NULL;
  
  GInetAddr *ia =  gnet_inetaddr_new(host, 0);
  if (ia != NULL)
    {
      ret = gnet_inetaddr_get_canonical_name(ia);
      gnet_inetaddr_delete(ia);
    }

  return ret;
}


//! Connects to the specified host.
SocketConnection *
GNetSocketDriver::connect(char *host, int port, void *data)
{
  GNetSocketConnection *con = new GNetSocketConnection;

  con->driver = this;
  con->data = data;
    
  gnet_tcp_socket_connect_async(host, port, static_async_connected, con);
  return con;
}


//! Listens at the specified port.
SocketConnection *
GNetSocketDriver::listen(int port, void *data)
{
  GNetSocketConnection *con = new GNetSocketConnection;

  con->driver = this;
  con->data = data;
  
#ifdef HAVE_GNET2
  con->socket = gnet_tcp_socket_server_new_with_port(port);
#else
  con->socket = gnet_tcp_socket_server_new(port);
#endif
  if (con->socket != NULL)
    {
      gnet_tcp_socket_server_accept_async(con->socket, static_async_accept, con);
    }
  else
    {
      delete con;
      con = NULL;
    }
  
  return con;
}


//! GNet has accepted a new connection.
void
GNetSocketDriver::async_accept(GTcpSocket *server, GTcpSocket *client, GNetSocketConnection *scon)
{
  (void) server;
  
  if (client != NULL)
    {
      GNetSocketConnection *ccon =  new GNetSocketConnection;

      ccon->driver = this;
      ccon->data = NULL;
#ifdef HAVE_GNET2
      ccon->iochannel = gnet_tcp_socket_get_io_channel(client);
#else
      ccon->iochannel = gnet_tcp_socket_get_iochannel(client);
#endif      
      ccon->watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
      ccon->socket = client;
      
      g_assert(ccon->iochannel);
      ccon->watch = g_io_add_watch(ccon->iochannel, (GIOCondition) ccon->watch_flags, static_async_io, ccon);

      if (listener != NULL)
        {
          listener->socket_accepted(scon, ccon);
        }
    }
}


//! GNets reports that data is ready to be read.
bool
GNetSocketDriver::async_io(GIOChannel *iochannel, GIOCondition condition, GNetSocketConnection *con)
{
  TRACE_ENTER("GNetSocketConnection::async_io");
  bool ret = true;

  g_assert(con != NULL);
  g_assert(iochannel != NULL);
  
  // check for socket error
  if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      if (listener != NULL)
        {
          con->close();
          listener->socket_closed(con, con->data);
        }
      ret = false;
    }

  // process input
  if (ret && (condition & G_IO_IN))
    {
      if (listener != NULL)
        {
          listener->socket_io(con, con->data);
        }
    }

  TRACE_EXIT();
  return ret;
}


//! GNet reports that the connection is established.
void 
GNetSocketDriver::async_connected(GTcpSocket *socket, GInetAddr *ia,
                                  GTcpSocketConnectAsyncStatus status,
                                  GNetSocketConnection *con)
{
  g_assert(con != NULL);
  
  if (status != GTCP_SOCKET_CONNECT_ASYNC_STATUS_OK)
    {
      gnet_tcp_socket_delete(socket);
      con->socket = NULL;

      if (listener != NULL)
        {
          listener->socket_closed(con, con->data);
        }
    }
  else
    {
      g_assert(ia != NULL);
      g_assert(socket != NULL);
      
      con->socket = socket;
      con->name = gnet_inetaddr_get_canonical_name(ia);
      con->port = gnet_inetaddr_get_port(ia);
#ifdef HAVE_GNET2      
      con->iochannel = gnet_tcp_socket_get_io_channel(socket);
#else
      con->iochannel = gnet_tcp_socket_get_iochannel(socket);
#endif      
      con->watch_flags = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
      con->watch = g_io_add_watch(con->iochannel, (GIOCondition)con->watch_flags, static_async_io, con);

      if (listener != NULL)
        {
          listener->socket_connected(con, con->data);
        }
      
      gnet_inetaddr_delete(ia);
    }
}


//! Accepted connection.
void
GNetSocketDriver::static_async_accept(GTcpSocket *server, GTcpSocket *client, gpointer data)
{
  GNetSocketConnection *con = (GNetSocketConnection *)data;
  g_assert(con != NULL);
  
  con->driver->async_accept(server, client, con);
}


//! IO ready.
gboolean
GNetSocketDriver::static_async_io(GIOChannel *iochannel, GIOCondition condition,
                                        gpointer data)
{
  GNetSocketConnection *con =  (GNetSocketConnection *)data;
  
  g_assert(con != NULL);
  return con->driver->async_io(iochannel, condition, con);
}


//! Connection established.
void 
GNetSocketDriver::static_async_connected(GTcpSocket *socket,
#ifndef HAVE_GNET2                                         
                                         GInetAddr *ia,
#endif                                         
                                         GTcpSocketConnectAsyncStatus status,
                                         gpointer data)
{
  GNetSocketConnection *con =  (GNetSocketConnection *)data;

#ifdef HAVE_GNET2
  GInetAddr *ia = NULL;
  if (socket != NULL)
    {
      ia = gnet_tcp_socket_get_remote_inetaddr(socket);
    }
#endif
  
  g_assert(con != NULL);
  con->driver->async_connected(socket, ia, status, con);
}


//! Creates a new connection.
GNetSocketConnection::GNetSocketConnection() :
  socket(NULL),
  iochannel(NULL),
  watch_flags(0),
  watch(0),
  driver(NULL),
  name(NULL),
  port(0)
{
}


//! Destructs the connection.
GNetSocketConnection::~GNetSocketConnection()
{
  TRACE_ENTER("GNetSocketConnection::~GNetSocketConnection");
#ifndef HAVE_GNET2
  if (iochannel != NULL)
    {
      // this causes troubles with gnet2
      g_io_channel_unref(iochannel);
    }
#endif
  
  if (socket != NULL)
    {
      TRACE_MSG("1");
      gnet_tcp_socket_delete(socket);
    }

  if (name != NULL)
    {
      g_free(name);
    }

  if (watch != 0)
    {
      g_source_remove(watch);
    }
  TRACE_EXIT();
}


//! Returns the canonical name of the remote host.
char *
GNetSocketConnection::get_canonical_name()
{
  return NULL;
}


//! Reads from the connection.
bool
GNetSocketConnection::read(void *buf, int count, int &bytes_read)
{
  bool ret = false;

  if (iochannel != NULL)
    {
      bytes_read = 0;
      
      GIOError error = g_io_channel_read(iochannel, (char *)buf, (gsize)count, (gsize *)&bytes_read);
  
      if (error != G_IO_ERROR_NONE)
        {
          bytes_read = -1;
        }
      else
        {
          ret = true;
        }
    }
  
  return ret;
}


//! Writes to the connection.
bool
GNetSocketConnection::write(void *buf, int count, int &bytes_written)
{
  bool ret = false;

  if (iochannel != NULL)
    {
      bytes_written = 0;
      GIOError error = g_io_channel_write(iochannel, (char *)buf, (gsize)count, (gsize *)&bytes_written);

      if (error != G_IO_ERROR_NONE)
        {
          bytes_written = -1;
        }
      else
        {
          ret = true;
        }
    }

  return ret;
}


//! Closes the connection.
bool
GNetSocketConnection::close()
{
  if (iochannel != NULL)
    {
      g_io_channel_unref(iochannel);
      iochannel = NULL;
    }

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
  return true;
}
