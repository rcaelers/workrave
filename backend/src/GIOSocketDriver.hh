//
// Copyright (C) 2002, 2003, 2007, 2008, 2009, 2010 Rob Caelers <robc@krandor.nl>
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

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "SocketDriver.hh"

using namespace workrave;

//! Listen socket implementation using GIO
class GIOSocketServer
  : public ISocketServer
{
public:
  GIOSocketServer();
  virtual ~GIOSocketServer();

  // ISocketServer  interface
  virtual void listen(int port);

private:
  static gboolean static_socket_incoming(GSocketService *service,
                                         GSocketConnection *connection,
                                         GObject *src_object,
                                         gpointer user_data);
  
  
private:
  GSocketService *service;
};


//! Socket implementation based on GIO
class GIOSocket
  : public ISocket
{
public:
  GIOSocket();
  GIOSocket(GSocketConnection *connection);
  virtual ~GIOSocket();

  // ISocket interface
  virtual void connect(const std::string &hostname, int port);
  virtual void read(void *buf, int count, int &bytes_read);
  virtual void write(void *buf, int count, int &bytes_written);
  virtual void close();

private:
  static void static_connected_callback(GObject *source_object,
                                        GAsyncResult *result,
                                        gpointer user_data);
  
  static gboolean static_data_callback(GSocket *socket,
                                   GIOCondition condition,
                                   gpointer user_data);
  
private:
  GSocketConnection *connection;
  GSocket *socket;
};


class GIOSocketDriver
  : public SocketDriver
{
  //! Create a new socket
  ISocket *create_socket();

  //! Create a new listen socket
  ISocketServer *create_server();
};

#endif // GIOSOCKETDRIVER_HH
