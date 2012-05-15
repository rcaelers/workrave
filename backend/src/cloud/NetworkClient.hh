// NetworkClient.hh
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

#ifndef NETWORKCLIENT_HH
#define NETWORKCLIENT_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "NetworkAddress.hh"

using namespace workrave;
using namespace workrave::network;

class NetworkClient
{
 public:
  typedef boost::shared_ptr<NetworkClient> Ptr;

  enum Scope
    {
      SCOPE_INVALID = 0,
      SCOPE_DIRECT = 1,
      SCOPE_MULTICAST = 2,
      SCOPE_CLOUD = 4
    };

  enum State
  {
    CONNECTION_STATE_INVALID,
    CONNECTION_STATE_CONNECTING,
    CONNECTION_STATE_CONNECTED,
    CONNECTION_STATE_CLOSED,
  };
  
 public:
  static Ptr create(Scope scope)
  {
    return NetworkClient::Ptr(new NetworkClient(scope));
  }

  NetworkClient(Scope scope) : scope(scope), state(CONNECTION_STATE_INVALID)
  {
  }

  bool authenticated;
  //WRID id;
  Scope scope;
  State state;
  NetworkAddress::Ptr address;
};
    
#endif // NETWORKCLIENT_HH
