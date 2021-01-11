//
// Copyright (C) 2002, 2003, 2007, 2008, 2010 Rob Caelers <robc@krandor.nl>
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

#ifndef GNETSOCKETDRIVER_HH
#define GNETSOCKETDRIVER_HH

#define GNET_EXPERIMENTAL
#include <glib.h>
#include <gnet.h>

#include "SocketDriver.hh"

using namespace workrave;

//! Listen socket implementation usinf GNet
class GNetSocketServer : public ISocketServer
{
public:
  GNetSocketServer();
  virtual ~GNetSocketServer();

  // ISocketServer  interface
  virtual void listen(int port);

private:
  // GNET callbacks
  void async_accept(GTcpSocket *server, GTcpSocket *client);
  static void static_async_accept(GTcpSocket *server, GTcpSocket *client, gpointer data);

private:
  //! GNet socket
  GTcpSocket *socket{nullptr};

  //! Glib IOChannel
  GIOChannel *iochannel{nullptr};

  //! I/O Events we are monitoring.
  gint watch_flags{0};

  //! Our watch ID
  guint watch{0};
};

//! Socket implementation based on GNet
class GNetSocket : public ISocket
{
public:
  GNetSocket();
  GNetSocket(GTcpSocket *socket);
  virtual ~GNetSocket();

  // ISocket interface
  virtual void connect(const std::string &hostname, int port);
  virtual void read(void *buf, int count, int &bytes_read);
  virtual void write(void *buf, int count, int &bytes_written);
  virtual void close();

private:
  // GNET callbacks
  bool async_io(GIOChannel *iochannel, GIOCondition condition);
  void async_connected(GTcpSocket *socket, GInetAddr *ia, GTcpSocketConnectAsyncStatus status);
  static gboolean static_async_io(GIOChannel *iochannel, GIOCondition condition, gpointer data);
  static void static_async_connected(GTcpSocket *socket, GTcpSocketConnectAsyncStatus status, gpointer data);

private:
  //! GNet socket
  GTcpSocket *socket{nullptr};

  //! Glib IOChannel
  GIOChannel *iochannel{nullptr};

  //! I/O Events we are monitoring
  gint watch_flags{0};

  //! Our watch ID
  guint watch{0};
};

class GNetSocketDriver : public SocketDriver
{
  //! Create a new socket
  ISocket *create_socket();

  //! Create a new listen socket
  ISocketServer *create_server();
};

#endif // GNETSOCKETDRIVER_HH
