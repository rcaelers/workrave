// NetworkMessage.cc
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <boost/shared_ptr.hpp>

#include "debug.hh"

#include "NetworkMessage.hh"

using namespace std;

NetworkMessageBase::Ptr
NetworkMessageBase::create(boost::shared_ptr<workrave::Header> header, boost::shared_ptr<google::protobuf::Message> message)
{
  Ptr ret = Ptr(new NetworkMessage<google::protobuf::Message>(message));
  ret->source = UUID::from_raw(header->source());
  return ret;
}
