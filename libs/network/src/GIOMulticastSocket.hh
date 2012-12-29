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

#ifndef GIOMULTICASTSOCKET_HH
#define GIOMULTICASTSOCKET_HH

#if defined(HAVE_GIO_NET)

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "Socket.hh"

#include "GIONetworkAddress.hh"

//! Socket implementation based on GIO
class GIOMulticastSocket
{
public:
  typedef boost::shared_ptr<GIOMulticastSocket> Ptr;
  
public:
  GIOMulticastSocket();
  virtual ~GIOMulticastSocket();

  bool join_multicast(const GIONetworkAddress::Ptr multicast_address, const std::string &adapter, const GIONetworkAddress::Ptr local_address);
  bool send(const gchar *buf, gsize count);
  bool receive(gchar *buf, gsize count, gsize &bytes_read, workrave::network::NetworkAddress::Ptr &address);
  void close();
  
  boost::signals2::signal<void()> &signal_io();
  
private:
  gboolean data_callback(GSocket *socket, GIOCondition condition);

  void watch_socket();

  static gboolean static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data);

private:
  GSocket *socket;

  //! Remote address
  GSocketAddress *remote_address;

  //! Adapter to use for multicast
  std::string adapter;

  //! Local address.
  GInetAddress *local_address;

  boost::signals2::signal<void()> io_signal;
};

#endif
#endif // GIOMULTICASTSOCKET_HH
