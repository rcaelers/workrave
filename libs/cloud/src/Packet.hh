// Packet.hh
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

#ifndef PACKET_HH
#define PACKET_HH

#include <google/protobuf/message.h>
#include "cloud.pb.h"

#include "Types.hh"
#include "cloud/Message.hh"
#include "cloud/MessageParams.hh"
#include "cloud/MessageContext.hh"

using namespace workrave::cloud;

class PacketIn
{
  PacketIn(Header::Ptr header, Message::Ptr message)
    : header(header), message(message)
  {
  }
  
public:
  typedef boost::shared_ptr<PacketIn> Ptr;

  static Ptr create(Header::Ptr header, Message::Ptr message)
  {
    return Ptr(new PacketIn(header, message));
  }

public:
  Header::Ptr header;
  Message::Ptr message;
  bool authentic;
  UUID source;
};

class PacketOut
{
  PacketOut(Message::Ptr message)
    : message(message)
  {
  }
  
public:
  typedef boost::shared_ptr<PacketOut> Ptr;

  static Ptr create(Message::Ptr message)
  {
    return Ptr(new PacketOut(message));
  }

public:
  Message::Ptr message;
  UUID source;
  bool sign;
};

#endif // PACKET_HH
