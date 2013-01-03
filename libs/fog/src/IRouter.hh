// IRouter.hh
//
// Copyright (C) 2007, 2008, 2009, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef IROUTER_HH
#define IROUTER_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "UUID.hh"

using namespace workrave::network;

namespace workrave
{
  namespace fog
  {
    struct ClientInfo
    {
      UUID id;
      UUID via;
    };
      
    class IRouter
    {
    public:
      typedef boost::shared_ptr<IRouter> Ptr;
      typedef boost::weak_ptr<IRouter> WeakPtr;
      
      virtual ~IRouter() {}
      
      virtual std::list<ClientInfo> get_client_infos() const = 0;
      virtual void connect(NetworkAddress::Ptr host, int port) = 0;
    };
  }
}


#endif // IROUTER_HH
