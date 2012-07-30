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

#ifndef GIOSERVERSOCKET_HH
#define GIOSERVERSOCKET_HH

#if defined(HAVE_GIO_NET)

#include <map>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <boost/signals2.hpp>

#include "SocketServer.hh"
#include "GIOSocket.hh"

//! Listen socket implementation using GIO
class GIOSocketServer : public workrave::network::SocketServer
{
public:
  typedef boost::shared_ptr<GIOSocketServer> Ptr;
  
public:
  GIOSocketServer();
  virtual ~GIOSocketServer();

  // SocketServer interface
  virtual bool init(int port, bool tls = false);
  boost::signals2::signal<void(workrave::network::Socket::Ptr)> &signal_accepted();

private:
  static gboolean static_socket_incoming(GSocketService *service,
                                         GSocketConnection *connection,
                                         GObject *src_object,
                                         gpointer user_data);

  void on_initialized(GIOSocket::WeakPtr socket);
  
private:
  GSocketService *service;
  bool tls;
  std::map<workrave::network::Socket::Ptr, boost::signals2::connection> waiting;
  boost::signals2::signal<void(workrave::network::Socket::Ptr)> accepted_signal;
};

#endif
#endif // GIOSERVERSOCKET_HH
