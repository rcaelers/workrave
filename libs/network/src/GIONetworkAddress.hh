// NetworkAddress.hh
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

#ifndef GIONETWORKADDRESS_HH
#define GIONETWORKADDRESS_HH

#include <string>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "network/NetworkAddress.hh"

class GIONetworkAddress : public workrave::network::NetworkAddress
{
public:
  typedef boost::shared_ptr<GIONetworkAddress> Ptr;

public:
  GIONetworkAddress();
  GIONetworkAddress(const std::string &ip, int port);
  GIONetworkAddress(GSocketAddress *address);
  GIONetworkAddress(GInetAddress *inet_address);
  GIONetworkAddress(const GIONetworkAddress &other);
  ~GIONetworkAddress();

  GSocketAddress *address() const;
  GInetAddress *inet_address() const;
  int port() const;
  GSocketFamily family() const;
  
  bool operator==(const NetworkAddress &other);
  GIONetworkAddress& operator=(const GIONetworkAddress &other);

  const std::string str();
  const std::string addr_str();
  
private:
  GSocketAddress *socket_address;
};

#endif
