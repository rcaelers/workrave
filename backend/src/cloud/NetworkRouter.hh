// NetworkRouter.hh --- Networking network server
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

#ifndef NETWORKROUTER_HH
#define NETWORKROUTER_HH

#include <list>
#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

#include <google/protobuf/message.h>

#include "NetworkAnnounce.hh"
#include "NetworkDirectLink.hh"
#include "WRID.hh"

#include "workrave.pb.h"

using namespace workrave;
using namespace workrave::network;

class NetworkRouter
{
public:
  enum Scope { SCOPE_DIRECT = 1, SCOPE_MULTICAST = 2, SCOPE_CLOUD = 4 };

  NetworkRouter(const WRID &my_id);
  virtual ~NetworkRouter();

  // Core internal
  void init(int port);
  void terminate();
  void heartbeat();

  void connect(const std::string &host, int port);
  void send_message(google::protobuf::Message &message, Scope scope);

private:
  const std::string marshall_message(google::protobuf::Message &message);
  bool unmarshall_message(gsize size, const gchar *data,
                          boost::shared_ptr<google::protobuf::Message> &result_message,
                          workrave::Header &result_header);

  std::string get_namespace_of_domain(int domain);

  void on_data(Scope scope, gsize size, const gchar *data, NetworkAddress::Ptr na);
  
private:
  //! My ID
  const WRID my_id;

  //! 
  NetworkAnnounce::Ptr announce;

  //! 
  NetworkDirectLink::Ptr direct_links;
};


#endif // NETWORKROUTER_HH
