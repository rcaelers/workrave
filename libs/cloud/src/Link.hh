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
#include <ostream>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "cloud/UUID.hh"

#include "Packet.hh"
#include "Marshaller.hh"

using namespace workrave;
using namespace workrave::network;

class Link : public boost::enable_shared_from_this<Link>
{
public:
  typedef boost::shared_ptr<Link> Ptr;
  typedef boost::weak_ptr<Link> WeakPtr;

  enum State
    {
      CONNECTION_STATE_INVALID,
      CONNECTION_STATE_CONNECTING,
      CONNECTION_STATE_CONNECTED,
      CONNECTION_STATE_CLOSED,
    };

  
public:
  Link();
  virtual ~Link();

  virtual void send_message(const std::string &message) = 0;
  
public:  
  State state;
  //NetworkAddress::Ptr address;
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

  NetworkAddress::Ptr address;
};

class ViaLink : public Link
{
public:
  typedef boost::shared_ptr<ViaLink> Ptr;
  
public:
  ViaLink(Link::Ptr link) : link(link) {}
  virtual ~ViaLink() {}
  
  static Ptr create(Link::Ptr link)
  {
    return ViaLink::Ptr(new ViaLink(link));
  }

  void send_message(const std::string &) {}
  
  Link::Ptr link;
  UUID id;
};

std::ostream& operator<< (std::ostream &out, Link *link);
std::ostream& operator<< (std::ostream &out, ViaLink *link);

#endif // LINK_HH
