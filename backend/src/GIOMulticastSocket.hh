//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

#include <sigc++/sigc++.h>

//! Multicast socket implementation based on GIO
class GIOMulticastSocket
{
public:
  GIOMulticastSocket(GSocketAddress *multicast_address, const std::string &adapter, GInetAddress *local_address);
  virtual ~GIOMulticastSocket();

  bool init();
  bool send(const gchar *buf, gsize count);
  sigc::signal<void, int, void *> &signal_multicast_data();

private:
  static gboolean static_data_callback(GSocket *socket, GIOCondition condition, gpointer user_data);

private:
  std::string adapter;
  GInetAddress *local_address;
  GSocketAddress *multicast_address;
  GSocket *socket;
  GSource *source;
  sigc::signal<void, int, void *> multicast_data_signal;
};

#endif
#endif // GIOMULTICASTSOCKET_HH
