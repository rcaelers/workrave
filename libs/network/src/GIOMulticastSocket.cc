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
#include "GIOMulticastSocket.hh"
#include "GIONetworkAddress.hh"

using namespace std;
using namespace workrave::network;

//! Creates a new connection.
GIOMulticastSocket::GIOMulticastSocket() :
  socket(NULL),
  port(0),
  remote_address(NULL),
  local_address(NULL)
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
      g_socket_shutdown(socket, TRUE, TRUE, NULL);
      g_socket_close(socket, NULL);
      g_object_unref(socket);
    }
      
  if (remote_address != NULL)
    {
      g_object_unref(remote_address);
    }
  
  TRACE_EXIT();
}



//! Connects to the specified host.
bool
GIOMulticastSocket::join_multicast(const GIONetworkAddress::Ptr multicast_address, const std::string &adapter, const GIONetworkAddress::Ptr local_address)
{
  TRACE_ENTER("GIOMulticastSocket::join_multicast");
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
      watch_socket();
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
GIOMulticastSocket::receive(gchar *buf, gsize count, gsize &bytes_read, NetworkAddress::Ptr &address)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::receive", count);

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
GIOMulticastSocket::send(const gchar *buf, gsize count)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::send", count);

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
  if (error != NULL)
    {
      g_error_free(error);
    }
  TRACE_EXIT();
}


boost::signals2::signal<void()> &
GIOMulticastSocket::signal_io()
{
  return io_signal;
}


gboolean
GIOMulticastSocket::data_callback(GSocket *socket, GIOCondition condition)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::static_data_callback", (int)condition);
  gboolean ret = TRUE;

  (void) socket;

  // process input
  if (ret && (condition & G_IO_IN))
    {
      io_signal();
    }

  TRACE_EXIT();
  return ret;
}

gboolean
GIOMulticastSocket::static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::static_data_callback", (int)condition);
  GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
  gboolean ret = self->data_callback(socket, condition);
  TRACE_EXIT();
  return ret;
}


void
GIOMulticastSocket::watch_socket()
{
  GSource *source = g_socket_create_source(socket, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP), NULL);
  g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
  g_source_attach(source, g_main_context_get_thread_default());
  g_source_unref(source);
}

#endif
