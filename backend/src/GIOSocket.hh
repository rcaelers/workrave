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

#ifndef GIOSOCKETDRIVER_HH
#define GIOSOCKETDRIVER_HH

#if defined(HAVE_GIO_NET)

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "ISocket.hh"

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
  void connect(GInetAddress *inet_addr, int port);

  static void static_connect_after_resolve(GObject *source_object, GAsyncResult *res, gpointer user_data);

  static void static_connected_callback(GObject *source_object,
                                        GAsyncResult *result,
                                        gpointer user_data);

  static gboolean static_data_callback(GSocket *socket,
                                   GIOCondition condition,
                                   gpointer user_data);

private:
  GSocketConnection *connection;
  GSocket *socket;
  GResolver *resolver;
  GSource *source;
  int port;
};

#endif
#endif // GIOSOCKETDRIVER_HH
