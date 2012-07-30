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

#ifndef GIOMULTICASTSERVER_HH
#define GIOMULTICASTSERVER_HH

#if defined(HAVE_GIO_NET)

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <boost/shared_ptr.hpp>

#include "MulticastSocketServer.hh"
#include "NetworkAddress.hh"
#include "NetworkInterfaceMonitor.hh"

#include "GIOMulticastSocket.hh"

//! 
class GIOMulticastSocketServer : public workrave::network::MulticastSocketServer
{
public:
  typedef boost::shared_ptr<GIOMulticastSocketServer> Ptr;
  
public:
  GIOMulticastSocketServer();
  virtual ~GIOMulticastSocketServer();

  // MulticastSocketServer interface
  virtual bool init(const std::string &address_ipv4, const std::string &address_ipv6, int port);
  virtual void send(const gchar *buf, gsize count);
  boost::signals2::signal<void(gsize, const gchar *, workrave::network::NetworkAddress::Ptr)> &signal_data();

private:
  class Connection
  {
  public:
    typedef boost::shared_ptr<Connection> Ptr;
    ~Connection()
    {
      if (local_address != NULL)
        {
          g_object_unref(local_address);
        }
    }
    std::string adapter_name;
    GInetAddress *local_address;
    GIOMulticastSocket::Ptr socket;
  };
  
private:
  void on_interface_changed(const NetworkInterfaceMonitor::NetworkInterfaceInfo &change);
  void on_data(Connection::Ptr connection);
  
private:
  GIONetworkAddress::Ptr address_ipv4;
  GIONetworkAddress::Ptr address_ipv6;
  NetworkInterfaceMonitor::Ptr monitor;
  std::list<Connection::Ptr> connections;
  boost::signals2::signal<void(gsize, const gchar *, workrave::network::NetworkAddress::Ptr)> data_signal;
};


#endif
#endif // GIOMULTICASTSERVER_HH
