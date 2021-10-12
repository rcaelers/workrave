//
// Copyright (C) 2002 - 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef GIOSOCKETDRIVER_HH
#define GIOSOCKETDRIVER_HH

#if defined(HAVE_GIO_NET) && defined(HAVE_DISTRIBUTION)

#  include <glib.h>
#  include <glib-object.h>
#  include <gio/gio.h>

#  include "SocketDriver.hh"

using namespace workrave;

//! Listen socket implementation using GIO
class GIOSocketServer : public ISocketServer
{
public:
  GIOSocketServer() = default;
  ~GIOSocketServer() override;

  // ISocketServer  interface
  void listen(int port) override;

private:
  static gboolean static_socket_incoming(GSocketService *service, GSocketConnection *connection, GObject *src_object, gpointer user_data);

private:
  GSocketService *service{nullptr};
};

//! Socket implementation based on GIO
class GIOSocket : public ISocket
{
public:
  GIOSocket();
  GIOSocket(GSocketConnection *connection);
  ~GIOSocket() override;

  // ISocket interface
  void connect(const std::string &hostname, int port) override;
  void read(void *buf, int count, int &bytes_read) override;
  void write(void *buf, int count, int &bytes_written) override;
  void close() override;

private:
  void connect(GInetAddress *inet_addr, int port);

  static void static_connect_after_resolve(GObject *source_object, GAsyncResult *res, gpointer user_data);

  static void static_connected_callback(GObject *source_object, GAsyncResult *result, gpointer user_data);

  static gboolean static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data);

private:
  GSocketConnection *connection{nullptr};
  GSocket *socket{nullptr};
  GResolver *resolver{nullptr};
  GSource *source{nullptr};
  int port{0};
};

class GIOSocketDriver : public SocketDriver
{
  //! Create a new socket
  ISocket *create_socket() override;

  //! Create a new listen socket
  ISocketServer *create_server() override;
};

#endif
#endif // GIOSOCKETDRIVER_HH
