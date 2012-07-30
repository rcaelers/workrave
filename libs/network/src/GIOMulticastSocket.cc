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
  //resolver(NULL),
  //socket_client(NULL),
  //connection(NULL),
  socket(NULL),
  //istream(NULL),
  //ostream(NULL),
  port(0),
  remote_address(NULL),
  local_address(NULL)
  //tls(true)
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
      
  // if (connection != NULL)
  //   {
  //     g_object_unref(connection);
  //   }

  // if (istream != NULL)
  //   {
  //     g_object_unref(istream);
  //   }

  // if (ostream != NULL)
  //   {
  //     g_object_unref(ostream);
  //   }

  // if (resolver != NULL)
  //   {
  //     g_object_unref(resolver);
  //   }

  // if (socket_client != NULL)
  //   {
  //     g_object_unref(socket_client);
  //   }

  if (remote_address != NULL)
    {
      g_object_unref(remote_address);
    }
  
  TRACE_EXIT();
}


// void
// GIOMulticastSocket::init(GSocketConnection *connection)
// {
//   TRACE_ENTER("GIOMulticastSocket::init");
//   socket = g_socket_connection_get_socket(connection);
//   g_object_ref(connection);

//   g_socket_set_blocking(socket, FALSE);
//   g_socket_set_keepalive(socket, TRUE);

//   watch_socket();

//   remote_address = g_socket_get_remote_address(socket, NULL);
//   TRACE_EXIT();
// }


// void
// GIOMulticastSocket::init_tls(GSocketConnection *connection)
// {
//   TRACE_ENTER("GIOMulticastSocket::init_tls");
//   GError *error = NULL;

//   GTlsCertificate *cert = g_tls_certificate_new_from_file("server-and-key.pem", &error);
//   g_assert_no_error(error);

//   GIOStream *tls_connection = g_tls_server_connection_new(G_IO_STREAM(connection), cert, &error);
//   g_assert_no_error(error);
//   g_object_unref(cert);

//   g_object_set(tls_connection, "authentication-mode", G_TLS_AUTHENTICATION_REQUIRED, NULL);
//   g_signal_connect(tls_connection, "accept-certificate", G_CALLBACK (static_accept_certificate), this);
 
//   g_tls_connection_handshake_async(G_TLS_CONNECTION(tls_connection), G_PRIORITY_DEFAULT, NULL, static_server_tls_handshake_callback, this);
//   TRACE_EXIT();
// }

// void
// GIOMulticastSocket::connect(const string &host_name, int port)
// {
//   TRACE_ENTER_MSG("GIOMulticastSocket::connect", host_name << " " << port);
//   this->port = port;
  
//   GInetAddress *inet_addr = g_inet_address_new_from_string(host_name.c_str());
  
//   if (inet_addr != NULL)
//     {
//       connect(g_inet_socket_address_new(inet_addr, port));
//       g_object_unref(inet_addr);
//     }
//   else
//     {
//       resolver = g_resolver_get_default();
//       g_resolver_lookup_by_name_async(resolver, host_name.c_str(), NULL, static_resolve_ready, this);
//     }
  
//   TRACE_EXIT();
// }


//! Connects to the specified host.
// void
// GIOMulticastSocket::connect(GSocketAddress *address)
// {
//   TRACE_ENTER("GIOMulticastSocket::connect");
//   this->remote_address = address;

//   socket_client = g_socket_client_new();

//   g_socket_client_connect_async(socket_client,
//                                 G_SOCKET_CONNECTABLE(remote_address),
//                                 NULL,
//                                 static_connected_callback,
//                                 this);

//   TRACE_EXIT();
// }


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
// bool
// GIOMulticastSocket::read(gchar *buf, gsize count, gsize &bytes_read)
// {
//   TRACE_ENTER_MSG("GIOMulticastSocket::read", count);

//   GError *error = NULL;
//   gsize num_read = 0;
//   bool ret = true;

//   if (socket != NULL)
//     {
//       num_read = g_socket_receive(socket, (char *)buf, count, NULL, &error);
//       if (error != NULL)
//         {
//           g_error_free(error);
//           num_read = 0;
//           ret = false;
//         }
//     }

//   bytes_read = (int)num_read;
//   TRACE_RETURN(bytes_read);
//   return ret;
// }



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


// //! Write to the connection.
// bool
// GIOMulticastSocket::write(const gchar *buf, gsize count)
// {
//   TRACE_ENTER_MSG("GIOMulticastSocket::write", count);

//   GError *error = NULL;
//   gsize num_written = 0;
//   if (socket != NULL)
//     {
//       num_written = g_socket_send(socket, (char *)buf, count, NULL, &error);
//       if (error != NULL)
//         {
//           g_error_free(error);
//           num_written = 0;
//         }
//     }
//   TRACE_RETURN(num_written);
//   return num_written == count;
// }


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

// boost::signals2::signal<void()> &
// GIOMulticastSocket::signal_connected()
// {
//   return connected_signal;
// }

// boost::signals2::signal<void()> &
// GIOMulticastSocket::signal_disconnected()
// {
//   return disconnected_signal;
// }


// void
// GIOMulticastSocket::client_tls_handshake()
// {
//   tlsconn;

//   else
//     {
//       enumerator_next_async (data);
//     }
// }


// void
// GIOMulticastSocket::connected_callback(GObject *source_object, GAsyncResult *result)
// {
//   TRACE_ENTER("GIOMulticastSocket::static_connected_callback");

//   GError *error = NULL;

//   GSocketConnection *socket_connection = g_socket_client_connect_finish(G_SOCKET_CLIENT(source_object), result, &error);

//   if (socket_connection != NULL)
//     {
//       if (tls)
//         {
//           GIOStream *tls_connection = g_tls_client_connection_new(socket_connection, NULL, &error);
//           if (tls_connection)
//             {
//               GTlsCertificateFlags flags;
//               flags = G_TLS_CERTIFICATE_VALIDATE_ALL &
//                 ~(G_TLS_CERTIFICATE_UNKNOWN_CA | G_TLS_CERTIFICATE_BAD_IDENTITY);

//               g_tls_client_connection_set_validation_flags(G_TLS_CLIENT_CONNECTION(tls_connection), flags);
//               g_tls_connection_handshake_async(G_TLS_CONNECTION(tls_connection),
//                                                G_PRIORITY_DEFAULT,
//                                                NULL,
//                                                static_client_tls_handshake_callback,
//                                                this);
//             }
//           else
//             {
//               connection = socket_connection;

//               socket = g_socket_connection_get_socket(connection);
//               g_socket_set_blocking(socket, FALSE);
//               g_socket_set_keepalive(socket, TRUE);

//               watch_socket()

//               connected_signal();
//             }
//         }
//     }
  
//   if (error != NULL)
//     {
//       TRACE_MSG("failed to connect");
//       g_error_free(error);
//     }

//   TRACE_EXIT();
// }


gboolean
GIOMulticastSocket::data_callback(GSocket *socket, GIOCondition condition)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::static_data_callback", (int)condition);
  gboolean ret = TRUE;

  (void) socket;

  // // check for socket error
  // if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
  //   {
  //     disconnected_signal();
  //     ret = FALSE;
  //   }

  // process input
  if (ret && (condition & G_IO_IN))
    {
      io_signal();
    }

  TRACE_EXIT();
  return ret;
}

// void
// GIOMulticastSocket::resolve_ready(GObject *source_object, GAsyncResult *res_data)
// {
//   TRACE_ENTER("GIOMulticastSocket::static_resolve_ready");
//   bool result = false;
//   GList *addresses = g_resolver_lookup_by_name_finish((GResolver *)source_object, res, NULL);

//   if (addresses != NULL)
//     {
//       // Take first result
//       if (addresses->data != NULL)
//         {
//           connect(g_inet_socket_address_new((GInetAddress *) addresses->data, port));
//           result = true;
//         }
//        g_resolver_free_addresses(addresses);
//     }

//   if (!result)
//     {
//       disconnected_signal();
//     }

//   TRACE_EXIT();
// }

// void
// GIOMulticastSocket::static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data)
// {
//   TRACE_ENTER("GIOMulticastSocket::static_connected_callback");
//   GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
//   self->connected_callback(source_object, result);
//   TRACE_EXIT();
// }

gboolean
GIOMulticastSocket::static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  TRACE_ENTER_MSG("GIOMulticastSocket::static_data_callback", (int)condition);
  GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
  gboolean ret = self->data_callback(socket, condition);
  TRACE_EXIT();
  return ret;
}

// void
// GIOMulticastSocket::static_resolve_ready(GObject *source_object, GAsyncResult *res, gpointer user_data)
// {
//   TRACE_ENTER("GIOMulticastSocket::static_resolve_ready");
//   GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
//   self->resolve_ready(source_object, res);
//   TRACE_EXIT();
// }

// gboolean
// GIOMulticastSocket::static_accept_certificate(GTlsClientConnection *conn, GTlsCertificate *cert,
//                                      GTlsCertificateFlags errors, gpointer user_data)
// {
//   GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
//   return errors == G_TLS_CERTIFICATE_UNKNOWN_CA;
// }


// void
// GIOMulticastSocket::server_tls_handshake_callback(GObject *object, GAsyncResult *result, gpointer user_data)
// {
//   TRACE_ENTER("GIOMulticastSocket::server_tls_handshake_callback");
//   GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
//   GError *error = NULL;
//   GOutputStream *stream;

//   g_tls_connection_handshake_finish(G_TLS_CONNECTION (object), res, &error);
//   g_assert_no_error(error);
//   TRACE_EXIT();
// }


// void
// GIOMulticastSocket::client_tls_handshake_callback(GObject *object, GAsyncResult *result, gpointer user_data)
// {
//   TRACE_ENTER("GIOMulticastSocket::client_tls_handshake_callback");
//   GIOMulticastSocket *self = (GIOMulticastSocket *)user_data;
//   GError *error = NULL;

//   if (g_tls_connection_handshake_finish(G_TLS_CONNECTION(object), result, &error))
//     {
//       self->connection = G_IO_STREAM(object);
//       self->connected_signal();
//     }
//   else
//     {
//       g_object_unref(object);
//     }
//   TRACE_EXIT();
// }


void
GIOMulticastSocket::watch_socket()
{
  GSource *source = g_socket_create_source(socket, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP), NULL);
  g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
  g_source_attach(source, g_main_context_get_thread_default());
  g_source_unref(source);
}


// void
// GIOMulticastSocket::watch_connection()
// {
//   GSource *source = g_pollable_input_stream_create_source(G_POLLABLE_INPUT_STREAM(g_io_stream_get_input_stream(stream)), NULL);
//   g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
//   g_source_attach(source, g_main_context_get_thread_default());
//   g_source_unref(source);
  
//   source = g_pollable_output_stream_create_source(G_POLLABLE_OUTPUT_STREAM (g_io_stream_get_output_stream (stream)), cancellable);
//   g_source_set_callback(source, (GSourceFunc) static_data_callback, (void*)this, NULL);
//   g_source_attach(source, g_main_context_get_thread_default());
//   g_source_unref(source);
// }

// NetworkAddress::Ptr
// GIOMulticastSocket::get_remote_address()
// {
//   return NetworkAddress::Ptr(new GIONetworkAddress(remote_address));
// }

#endif
