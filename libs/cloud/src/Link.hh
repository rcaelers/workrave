// Link.hh
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

#ifndef LINK_HH
#define LINK_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "cloud/UUID.hh"

#include "Packet.hh"
#include "Marshaller.hh"

using namespace workrave;
using namespace workrave::network;

class Link
{
public:
  typedef boost::shared_ptr<Link> Ptr;

  enum State
    {
      CONNECTION_STATE_INVALID,
      CONNECTION_STATE_CONNECTING,
      CONNECTION_STATE_CONNECTED,
      CONNECTION_STATE_CLOSED,
    };

  
public:
  static Ptr create();

  Link();
  virtual ~Link();

  virtual void send_message(const std::string &message) = 0;
  
private:

public:  
  bool authenticated;
  UUID id;
  State state;
  NetworkAddress::Ptr address;
};

class EphemeralLink : public Link
{
public:
  typedef boost::shared_ptr<EphemeralLink> Ptr;
  
public:
  EphemeralLink() {}
  virtual ~EphemeralLink() {}
  
  static Ptr create()
  {
    return EphemeralLink::Ptr(new EphemeralLink());
  }

  void send_message(const std::string &) {}
};

class ViaLink : public Link
{
public:
  typedef boost::shared_ptr<ViaLink> Ptr;
  
public:
  ViaLink(Link::Ptr via) : via(via) {}
  virtual ~ViaLink() {}
  
  static Ptr create(Link::Ptr via)
  {
    return ViaLink::Ptr(new ViaLink(via));
  }

  void send_message(const std::string &) {}
  
  Link::Ptr via;
};

#endif // LINK_HH
