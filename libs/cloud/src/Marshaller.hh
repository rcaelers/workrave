// Marshaller.hh
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

#ifndef MARSHALLER_HH
#define MARSHALLER_HH

#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include "network/NetworkAddress.hh"
#include "cloud/UUID.hh"

#include "Packet.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::cloud;

class Marshaller
{
public:
  typedef boost::shared_ptr<Marshaller> Ptr;

public:
  static Ptr create();

  Marshaller();
  virtual ~Marshaller();

  void set_id(UUID &id);
  void set_credentials(const std::string &username, const std::string &secret);
  
  PacketIn::Ptr unmarshall(gsize size, const gchar *data);
  const std::string marshall(PacketOut::Ptr message);
  
private:
  std::string get_namespace_of_domain(int domain);
  int get_domain_of_namespace(const std::string &ns);
  
  const std::string get_nonce() const;
  bool check_authentication(Header::Ptr header);
  void add_authentication(Header::Ptr header);

public:  
  UUID myid;
  std::string username;
  std::string secret;
};

  
#endif // MARSHALLER_HH
