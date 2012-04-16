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
#include "SocketException.hh"

using namespace std;


//! Creates a new listen socket.
GIOSocketServer::GIOSocketServer() :
  service(NULL)
{
}


//! Destructs the listen socket.
GIOSocketServer::~GIOSocketServer()
{
  if (service != NULL)
    {
      g_socket_service_stop(service);
      g_object_unref(service);
      service = NULL;
    }
}


//! Listen at the specified port.
void
GIOSocketServer::listen(int port)
{
  GError *error = NULL;

  service = g_socket_service_new();
  if (service == NULL)
    {
      throw SocketException("Failed to create server");
    }

  gboolean rc = g_socket_listener_add_inet_port(G_SOCKET_LISTENER(service), port, NULL, &error);
  if (!rc)
    {
      g_object_unref(service);
      service = NULL;

      throw SocketException(string("Failed to listen: ") + error->message);
    }

  g_signal_connect(service, "incoming", G_CALLBACK(static_socket_incoming), this);
  g_socket_service_start(service);

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

  try
    {
      GIOSocketServer *ss = (GIOSocketServer *) user_data;
      GIOSocket *socket =  new GIOSocket(connection);
      ss->listener->socket_accepted(ss, socket);
    }
  catch(...)
    {
      // Make sure that no exception reach the glib mainloop.
    }

  TRACE_EXIT();
	return FALSE;
}

#endif
