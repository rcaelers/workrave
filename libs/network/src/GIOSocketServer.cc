// GIOSocketServer.cc
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
#include "GIOSocketServer.hh"
#include "GIOSocket.hh"

using namespace std;
using namespace workrave::network;

//! Creates a new listen socket.
GIOSocketServer::GIOSocketServer() :
  service(NULL)
{
}


//! Destructs the listen socket.
GIOSocketServer::~GIOSocketServer()
{
  TRACE_ENTER("GIOSocketServer::~GIOSocketServer");
  if (service != NULL)
    {
      g_socket_listener_close(G_SOCKET_LISTENER(service));
      g_socket_service_stop(service);
      g_object_unref(service);
      service = NULL;
      TRACE_MSG("stopping");
    }
  TRACE_EXIT();
}


//! Listen at the specified port.
bool
GIOSocketServer::init(int port, bool tls)
{
  TRACE_ENTER_MSG("GIOSocketServer::init", port);
  this->tls = tls;
  
  GError *error = NULL;
  service = g_socket_service_new();
  
  g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), port, NULL, &error);
  if (error != NULL)
    {
      TRACE_MSG("error" << error->message);
      g_error_free(error);
      g_object_unref(service);
      service = NULL;
      return false;
    }

  g_signal_connect(service, "incoming", G_CALLBACK(static_socket_incoming), this);
  g_socket_service_start(service);

  TRACE_EXIT();
  return true;
}


boost::signals2::signal<void(Socket::Ptr)> &
GIOSocketServer::signal_accepted()
{
  return accepted_signal;
}


gboolean
GIOSocketServer::static_socket_incoming(GSocketService *service,
                                        GSocketConnection *connection,
                                        GObject *src_object,
                                        gpointer user_data)
{
  TRACE_ENTER("GIOSocketServer::static_socket_incoming");
  (void) service;
  (void) src_object;

  GIOSocketServer *self = (GIOSocketServer *) user_data;
  GIOSocket::Ptr socket =  GIOSocket::Ptr(new GIOSocket());

  boost::signals2::connection c = socket->signal_connection_state_changed()
    .connect(boost::bind(&GIOSocketServer::on_initialized, self, GIOSocket::WeakPtr(socket)));
  
  self->waiting[socket] = c;
  
  TRACE_MSG(string("use ") << socket.use_count());
  socket->init(connection, self->tls);
  TRACE_MSG(string("use ") << socket.use_count());

  TRACE_EXIT();
	return FALSE;
}

void
GIOSocketServer::on_initialized(GIOSocket::WeakPtr weak_socket)
{
  GIOSocket::Ptr socket = weak_socket.lock();
    
  TRACE_ENTER("GIOSocketServer::on_initialized");
  TRACE_MSG(string("use ") << socket.use_count());
  boost::signals2::connection c = waiting[socket];
  c.disconnect();
  TRACE_MSG(string("use ") << socket.use_count());
  waiting.erase(socket);
  TRACE_MSG(string("use ") << socket.use_count());
  if (socket->is_connected())
    {
      accepted_signal(socket);
    }
  TRACE_EXIT();
}


#endif
