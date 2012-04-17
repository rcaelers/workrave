// NetworkAnnounce.hh --- Network Announcements
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

#ifndef NETWORKANNOUNCE_HH
#define NETWORKANNOUNCE_HH

#include <string>

#include "WRID.hh"

// Forward declarion of external interfaces.
namespace workrave
{
  class WRID;
}
using namespace workrave;


// Forward declarion of internal interfaces.
class IMulticastServer;

//! Main entry point of all networking functionality.
class NetworkAnnounce
{
public:
  NetworkAnnounce(const WRID &my_id);
  virtual ~NetworkAnnounce();

  // Core internal
  void init();
  void terminate();
  void heartbeat();

private:
  void on_multicast_data(int size, void *data);

private:
  //! My ID
  WRID my_id;

  //!
  IMulticastServer *multicast_server;
};

#endif // NETWORKANNOUNCE_HH
