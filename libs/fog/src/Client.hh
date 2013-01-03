// Client.hh
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

#ifndef CLIENT_HH
#define CLIENT_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "Link.hh"
#include "UUID.hh"

using namespace workrave;
using namespace workrave::network;

class Client
{
public:
  typedef boost::shared_ptr<Client> Ptr;

public:
  static Ptr create()
  {
    return Ptr(new Client());
  }

  Client() : authenticated(false) {}
  virtual ~Client()
  {
    TRACE_ENTER("Client::~Client");
    TRACE_EXIT();
  }

public:
  Link::Ptr link;
  bool authenticated;
  UUID id;
};

std::ostream& operator<< (std::ostream &out, Client *client);

#endif // CLIENT_HH
