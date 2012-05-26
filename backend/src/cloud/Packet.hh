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
#include "workrave.pb.h"

#include "Types.hh"
#include "MessageParams.hh"
#include "MessageContext.hh"

class Packet
{
  Packet(workrave::cloud::Header::Ptr header, workrave::cloud::Message::Ptr message)
    : header(header), message(message) { }
  
  Packet(workrave::cloud::Header::Ptr header, workrave::cloud::Message::Ptr message, workrave::cloud::MessageParams::Ptr params)
    : header(header), message(message), params(params) { }
  
public:
  typedef boost::shared_ptr<Packet> Ptr;

  static Ptr create(workrave::cloud::Header::Ptr header, workrave::cloud::Message::Ptr message)
  {
    return Ptr(new Packet(header, message));
  }

  static Ptr create(workrave::cloud::Message::Ptr message, workrave::cloud::MessageParams::Ptr params)
  {
    workrave::cloud::Header::Ptr header(new workrave::cloud::proto::Header);
    
    return Ptr(new Packet(header, message, params));
  }
  
public:
  workrave::cloud::Header::Ptr header;
  workrave::cloud::Message::Ptr message;
  workrave::cloud::MessageParams::Ptr params;
  workrave::cloud::MessageContext::Ptr context;
};

#endif // PACKET_HH
