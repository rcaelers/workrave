// INetwork.hh
//
// Copyright (C) 2007, 2008, 2009, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef INETWORK_HH
#define INETWORK_HH

#include <list>
#include <map>
#include <string>

#include <sigc++/sigc++.h>

#include <boost/shared_ptr.hpp>
#include <google/protobuf/message.h>

#include "NetworkMessage.hh"
#include "workrave.pb.h"

using namespace workrave;

class INetwork
{
public:
  typedef boost::shared_ptr<INetwork> Ptr;
  typedef sigc::signal<void, NetworkMessageBase::Ptr> MessageSignal;

  virtual ~INetwork() {}

  virtual void send_message(NetworkMessageBase::Ptr msg) = 0;
  virtual MessageSignal &signal_message(int domain, int id) = 0;
};


#endif // INETWORK_HH
