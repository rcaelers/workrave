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
  socket(NULL),
  iostream(NULL),
  istream(NULL),
  ostream(NULL),
  source(NULL),
  port(0),
  remote_address(NULL),
  local_address(NULL),
  tls(false),
  connected(false)
{
  TRACE_ENTER("GIOSocket::GIOSocket()");

  GTlsBackend *b = g_tls_backend_get_default();
  TRACE_MSG(g_tls_backend_supports_tls(b));
  TRACE_EXIT();
}


//! Destructs the connection.
GIOSocket::~GIOSocket()
{
  TRACE_ENTER("GIOSocket::~GIOSocket");
  TRACE_MSG(port);

  if (source != NULL)
    {
      g_source_destroy(source);
    }
  
  if (socket != NULL)
    {
      g_socket_shutdown(socket, TRUE, TRUE, NULL);
      g_socket_close(socket, NULL);
    }
      
  if (iostream != NULL)
    {
      g_object_unref(iostream);
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
GIOSocket::init(GSocketConnection *connection, bool tls)
{
  TRACE_ENTER_MSG("GIOSocket::init", tls);
  this->tls = tls;
  
  socket = g_socket_connection_get_socket(connection);
  g_socket_set_blocking(socket, FALSE);
  g_socket_set_keepalive(socket, TRUE);
  remote_address = g_socket_get_remote_address(socket, NULL);

  if (!tls)
    {
      iostream = G_IO_STREAM(connection);
      g_object_ref(iostream);

      prepare_connection();
      connected = true;
      connection_state_changed_signal();
    }
  else
    {
      TRACE_MSG("TLS");
      GError *error = NULL;

      GTlsDatabase *database = g_tls_file_database_new("/home/robc/ca.pem", &error);
      g_assert_no_error(error);

      GTlsCertificate *cert = g_tls_certificate_new_from_file("/home/robc/server-keys.pem", &error);
      g_assert_no_error(error);

      iostream = g_tls_server_connection_new(G_IO_STREAM(connection), cert, &error);
      g_assert_no_error(error);
      g_object_unref(cert);

      //prepare_connection();
      
      g_tls_connection_set_database(G_TLS_CONNECTION(iostream), database);
      g_object_set(iostream, "authentication-mode", G_TLS_AUTHENTICATION_REQUIRED, NULL);
      g_signal_connect(iostream, "accept-certificate", G_CALLBACK(static_accept_certificate), this);
      g_tls_connection_handshake_async(G_TLS_CONNECTION(iostream), G_PRIORITY_DEFAULT, NULL, static_tls_handshake_callback, this);
      ref();
    }
  TRACE_EXIT();
}

void
GIOSocket::connect(const string &host_name, int port, bool tls)
{
  TRACE_ENTER_MSG("GIOSocket::connect", host_name << " " << port << " " <<  tls);
  this->port = port;
  this->tls = tls;
  
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
      ref();
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
  ref();

  TRACE_EXIT();
}


//! Read from the connection.
bool
GIOSocket::read(gchar *buf, gsize count, gsize &bytes_read)
{
  TRACE_ENTER_MSG("GIOSocket::read", count);

  GError *error = NULL;
  gssize num_read = 0;
  bool ret = true;

  if (istream != NULL)
    {
      num_read = g_pollable_input_stream_read_nonblocking(G_POLLABLE_INPUT_STREAM(istream),
                                                          (char *)buf, count, NULL, &error);

      if (num_read > 0)
        {
          TRACE_MSG("num_read > 0 : " <<  num_read);
          bytes_read = (int) num_read;
        }
      else if (num_read == 0)
        {
          TRACE_MSG("num_read == 0");
          bytes_read = (int) num_read;
          ret = false;
        }
      else if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
        {
          TRACE_MSG("would block");
          bytes_read = 0;
        }
      else
        {
          TRACE_MSG("error");
          ret = false;
        }
    }

  TRACE_GERROR(error);
  g_clear_error(&error);
  
  TRACE_RETURN(bytes_read);
  return ret;
}


//! Write to the connection.
bool
GIOSocket::write(const gchar *buf, gsize count)
{
  TRACE_ENTER_MSG("GIOSocket::write", count);

  GError *error = NULL;
  gssize num_written = 0;
  if (ostream != NULL)
    {
      num_written = g_pollable_output_stream_write_nonblocking(G_POLLABLE_OUTPUT_STREAM(ostream),
                                                               (char *)buf, count, NULL, &error);
    }

  TRACE_GERROR(error);
  g_clear_error(&error);

  TRACE_RETURN(num_written);
  return num_written == (gssize)count;
}


//! Close the connection.
void
GIOSocket::close()
{
  TRACE_ENTER("GIOSocket::close");
  TRACE_MSG(port);
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


GIOSocket::io_signal_type &
GIOSocket::signal_io()
{
  return io_signal;
}


GIOSocket::connection_state_changed_signal_type &
GIOSocket::signal_connection_state_changed()
{
  return connection_state_changed_signal;
}


void
GIOSocket::connected_callback(GObject *source_object, GAsyncResult *result)
{
  TRACE_ENTER("GIOSocket::connected_callback");
  TRACE_MSG(port);

  GError *error = NULL;
  GSocketConnection *connection = g_socket_client_connect_finish(G_SOCKET_CLIENT(source_object), result, &error);
  if (connection != NULL)
    {
      if (tls)
        {
          iostream = g_tls_client_connection_new(G_IO_STREAM(connection), NULL, &error);
          if (iostream)
            {
              socket = g_socket_connection_get_socket(connection);
              g_socket_set_blocking(socket, FALSE);
              g_socket_set_keepalive(socket, TRUE);

              GTlsDatabase *database = g_tls_file_database_new("/home/robc/ca.pem", &error);
              g_assert_no_error(error);
              
              //prepare_connection();
              GTlsCertificate *cert = g_tls_certificate_new_from_file("/home/robc/client-keys.pem", &error);
              g_assert_no_error(error);
              g_tls_connection_set_certificate(G_TLS_CONNECTION(iostream), cert);
              
              GTlsCertificateFlags flags;
              flags = (GTlsCertificateFlags) (G_TLS_CERTIFICATE_VALIDATE_ALL); // &
              //                                              ~(G_TLS_CERTIFICATE_UNKNOWN_CA | G_TLS_CERTIFICATE_BAD_IDENTITY));

              g_tls_connection_set_database(G_TLS_CONNECTION(iostream), database);
              g_tls_client_connection_set_validation_flags(G_TLS_CLIENT_CONNECTION(iostream), flags);
              g_tls_connection_handshake_async(G_TLS_CONNECTION(iostream),
                                               G_PRIORITY_DEFAULT,
                                               NULL,
                                               static_tls_handshake_callback,
                                               this);
              ref();
              g_object_unref(connection);
            }
        }
      else
        {
          iostream = G_IO_STREAM(connection);
          
          socket = g_socket_connection_get_socket(connection);
          g_socket_set_blocking(socket, FALSE);
          g_socket_set_keepalive(socket, TRUE);

          prepare_connection();
          connected = true;
          connection_state_changed_signal();
        }
    }

  TRACE_GERROR(error);
  g_clear_error(&error);
  unref();

  TRACE_EXIT();
}


gboolean
GIOSocket::data_callback(GObject *pollable)
{
  TRACE_ENTER("GIOSocket::data_callback");
  TRACE_MSG(port);

  io_signal();

  TRACE_EXIT();
  return TRUE;
}


void
GIOSocket::resolve_ready(GObject *source_object, GAsyncResult *res)
{
  TRACE_ENTER("GIOSocket::resolve_ready");
  bool result = false;
  GList *addresses = g_resolver_lookup_by_name_finish((GResolver *)source_object, res, NULL);

  if (addresses != NULL)
    {
      // Take first result
      if (addresses->data != NULL)
        {
          connect(g_inet_socket_address_new((GInetAddress *) addresses->data, port));
          result = true;
        }
       g_resolver_free_addresses(addresses);
    }

  if (!result)
    {
      connected = false;
      connection_state_changed_signal();      
    }

  unref();
  TRACE_EXIT();
}


void
GIOSocket::static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data)
{
  GIOSocket *self = (GIOSocket *)user_data;
  self->connected_callback(source_object, result);
}


gboolean
GIOSocket::static_data_callback(GObject *pollable, gpointer user_data)
{
  GIOSocket *self = (GIOSocket *)user_data;
  return self->data_callback(pollable);
}


void
GIOSocket::static_resolve_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  GIOSocket *self = (GIOSocket *)user_data;
  self->resolve_ready(source_object, res);
}


gboolean
GIOSocket::static_accept_certificate(GTlsClientConnection *conn, GTlsCertificate *cert,
                                     GTlsCertificateFlags errors, gpointer user_data)
{
  TRACE_ENTER_MSG("GIOSocket::static_accept_certificate", errors);
  TRACE_EXIT()
  //GIOSocket *self = (GIOSocket *)user_data;
  return errors == 0;
}


void
GIOSocket::static_tls_handshake_callback(GObject *object, GAsyncResult *result, gpointer user_data)
{
  TRACE_ENTER("GIOSocket::static_tls_handshake_callback");
  GIOSocket *self = (GIOSocket *)user_data;
  GError *error = NULL;

  TRACE_MSG(self->port);
  
  self->connected = g_tls_connection_handshake_finish(G_TLS_CONNECTION(object), result, &error);
  TRACE_MSG("connected " << self->connected);
  TRACE_GERROR(error);

  if (self->connected)
    {
      self->prepare_connection();
    }

  self->connection_state_changed_signal();

  g_clear_error(&error);
  self->unref();
  TRACE_EXIT();
}


void
GIOSocket::prepare_connection()
{
  istream = g_io_stream_get_input_stream(iostream);
  ostream = g_io_stream_get_output_stream(iostream);

  source = g_pollable_input_stream_create_source(G_POLLABLE_INPUT_STREAM(istream), NULL);
  g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
  g_source_attach(source, g_main_context_get_thread_default());
  g_source_unref(source);
}


NetworkAddress::Ptr
GIOSocket::get_remote_address()
{
  return NetworkAddress::Ptr(new GIONetworkAddress(remote_address));
}


bool
GIOSocket::is_connected() const
{
  return connected;
}

#endif
