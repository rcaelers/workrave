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

#include <sigc++/sigc++.h>

#include "Socket.hh"

#include "GIONetworkAddress.hh"

//! Socket implementation based on GIO
class GIOSocket
  : public workrave::network::Socket
{
public:
  typedef boost::shared_ptr<GIOSocket> Ptr;
  
public:
  GIOSocket();
  virtual ~GIOSocket();

  void init(GSocketConnection *connection);
  
  // Socket interface
  virtual void connect(const std::string &host_name, int port);
  virtual bool join_multicast(const GIONetworkAddress::Ptr multicast_address, const std::string &adapter, const GIONetworkAddress::Ptr local_address);
  virtual bool read(gchar *data, gsize count, gsize &bytes_read);
  virtual bool write(const gchar *data, gsize count);
  virtual bool send(const gchar *buf, gsize count);
  virtual bool receive(gchar *buf, gsize count, gsize &bytes_read, workrave::network::NetworkAddress::Ptr &address);
  virtual void close();
  virtual workrave::network::NetworkAddress::Ptr get_remote_address();
  
  virtual sigc::signal<void> &signal_io();
  virtual sigc::signal<void> &signal_connected();
  virtual sigc::signal<void> &signal_disconnected();
  
private:
  void connect(GSocketAddress *address);

  void on_resolve_ready(bool result);

  static void static_resolve_ready(GObject *source_object, GAsyncResult *res, gpointer user_data);
  static void static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data);
  static gboolean static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data);

private:
  GResolver *resolver;
  GSocketClient *socket_client;
  GSocketConnection *connection;
  GSocket *socket;
  GSource *source;
  int port;

  //! Remote address
  GSocketAddress *remote_address;

  //! Adapter to use for multicast
  std::string adapter;

  //! Local address.
  GInetAddress *local_address;

  sigc::signal<void> io_signal;
  sigc::signal<void> connected_signal;
  sigc::signal<void> disconnected_signal;
};

#endif
#endif // GIOSOCKET_HH
