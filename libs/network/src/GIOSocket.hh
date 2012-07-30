//
// Copyright (C) 2002, 2003, 2007, 2008, 2009, 2010, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef GIOSOCKET_HH
#define GIOSOCKET_HH

#if defined(HAVE_GIO_NET)

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>


#include <boost/enable_shared_from_this.hpp>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "utils/Object.hh"

#include "Socket.hh"

#include "GIONetworkAddress.hh"

//! Socket implementation based on GIO
class GIOSocket
  : public workrave::network::Socket,
    public workrave::utils::Object // ,
    //public boost::enable_shared_from_this<GIOSocket>
{
public:
  typedef boost::shared_ptr<GIOSocket> Ptr;
  typedef boost::weak_ptr<GIOSocket> WeakPtr;

public:
  GIOSocket();
  virtual ~GIOSocket();

  void init(GSocketConnection *connection, bool tls);
  
  // Socket interface
  virtual void connect(const std::string &host_name, int port, bool tls);
  virtual bool is_connected() const;
  virtual bool read(gchar *data, gsize count, gsize &bytes_read);
  virtual bool write(const gchar *data, gsize count);
  virtual void close();
  virtual workrave::network::NetworkAddress::Ptr get_remote_address();
  
  virtual io_signal_type &signal_io();
  virtual connection_state_changed_signal_type &signal_connection_state_changed();

private:
  void connect(GSocketAddress *address);

  void on_resolve_ready(bool result);

  void resolve_ready(GObject *source_object, GAsyncResult *res);
  void connected_callback(GObject *source_object, GAsyncResult *result);
  gboolean data_callback(GObject *pollable);
  gboolean accept_certificate (GTlsClientConnection *conn, GTlsCertificate *cert, GTlsCertificateFlags errors);

  void prepare_connection();
  
  static void static_resolve_ready(GObject *source_object, GAsyncResult *res, gpointer user_data);
  static void static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data);
  static gboolean static_data_callback(GObject *pollable, gpointer data);
  static gboolean static_accept_certificate(GTlsClientConnection *conn, GTlsCertificate *cert, GTlsCertificateFlags errors, gpointer user_data);
  static void static_tls_handshake_callback(GObject *object, GAsyncResult *result, gpointer user_data);


private:
  GResolver *resolver;
  GSocketClient *socket_client;
  GSocket *socket;
  GIOStream *iostream;
  GInputStream *istream;
  GOutputStream *ostream;
  GSource *source;
  int port;

  //! Remote address
  GSocketAddress *remote_address;

  //! Local address.
  GInetAddress *local_address;

  //!
  bool tls;

  //!
  bool connected;
  
  io_signal_type io_signal;
  connection_state_changed_signal_type connection_state_changed_signal;
};

#endif
#endif // GIOSOCKET_HH
