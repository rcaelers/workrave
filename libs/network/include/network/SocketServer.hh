// SocketServer.hh
//
// Copyright (C) 2002 - 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef SOCKETSERVER_HH
#define SOCKETSERVER_HH

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Socket.hh"

namespace workrave
{
  namespace network
  {
    class SocketServer
    {
    public:
      typedef boost::shared_ptr<SocketServer> Ptr;
  
    public:
      static Ptr create();

      virtual ~SocketServer() {};
     
      virtual bool init(int port) = 0;
      virtual boost::signals2::signal<void(Socket::Ptr)> &signal_accepted() = 0;
    };
  }
}

#endif // SOCKETSERVER_HH
