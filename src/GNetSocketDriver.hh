//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef GNETSOCKETDRIVER_HH
#define GNETSOCKETDRIVER_HH

#include <glib.h>
#include <gnet/gnet.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "SocketDriver.hh"

class GNetSocketDriver;

class GNetSocketConnection : public SocketConnection
{
public:
  GNetSocketConnection();
  virtual ~GNetSocketConnection();

  char *get_canonical_name();
  bool read(void *buf, int count, int &bytes_read);
  bool write(void *buf, int count, int &bytes_written);
  bool close();
  
private:
  //! GNet socket
  GTcpSocket *socket;

  //! Glib IOChannel.
  GIOChannel *iochannel;
  
  //! I/O Events we are monitoring.
  gint watch_flags;

  //! Our Watch
  guint watch;

  //! for static methods.
  GNetSocketDriver *driver;

  //! Canonical host name.
  gchar *name;

  //! Port connected to.
  gint port;
  
  friend class GNetSocketDriver;
};


class GNetSocketDriver : public SocketDriver
{
public:
  GNetSocketDriver();
  virtual ~GNetSocketDriver();

  char *get_my_canonical_name();
  char *canonicalize(char *);
  
  bool init();
  SocketConnection *connect(char *hostname, int port, void *data);
  SocketConnection *listen(int port, void *data);
  
private:
  void async_accept(GTcpSocket *server, GTcpSocket *client, GNetSocketConnection *c);
  bool async_io(GIOChannel* iochannel, GIOCondition condition, GNetSocketConnection *c);
  void async_connected(GTcpSocket *socket, GInetAddr *ia, GTcpSocketConnectAsyncStatus status,
                       GNetSocketConnection *c);
  
  static void static_async_accept(GTcpSocket* server, GTcpSocket* client, gpointer data);
  static gboolean static_async_io(GIOChannel* iochannel, GIOCondition condition, gpointer data);
  static void static_async_connected(GTcpSocket *socket, GInetAddr *ia,
                                     GTcpSocketConnectAsyncStatus status, gpointer data);
};

#endif // GNETSOCKETDRIVER_HH
