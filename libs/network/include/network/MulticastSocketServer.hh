// MulticastSocketServer.hh
//
// Copyright (C) 2002, 2003, 2005, 2007, 2010, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef MULTICASTSERVER_HH
#define MULTICASTSERVER_HH

#include <string>

#include <glib.h>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "NetworkAddress.hh"

namespace workrave
{
  namespace network
  {
    //! Multicast Server.
    class MulticastSocketServer
    {
    public:
      typedef boost::shared_ptr<MulticastSocketServer> Ptr;
  
    public:
      static Ptr create();

      virtual ~MulticastSocketServer() {};

      //! Initializes multicast server.
      virtual bool init(const std::string &address_ipv4, const std::string &address_ipv6, int port) = 0;

      //! Send data.
      virtual void send(const gchar *buf, gsize count) = 0;

      //! Incoming multicast data event.
      virtual boost::signals2::signal<void(gsize, const gchar *, NetworkAddress::Ptr)> &signal_data() = 0;
    };
  }
}

#endif // MULTICASTSERVER_HH
