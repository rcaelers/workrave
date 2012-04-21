// Network.hh --- Networking link server
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

#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>

#include "INetwork.hh"
#include "IConfiguratorListener.hh"

#include "WRID.hh"

// Forward declarion of external interfaces.
namespace workrave
{
  class WRID;
  class ICore;
}
using namespace workrave;


// Forward declarion of internal interfaces.
class NetworkAnnounce;

//! Main entry point of all networking functionality.
class Network
  : public INetwork
{
public:
  Network();
  virtual ~Network();

  // Core internal
  void init();
  void terminate();
  void heartbeat();

private:
  void init_my_id();

private:
  //! My ID
  WRID my_id;

  //! Workrave announcer
  NetworkAnnounce *announcer;
  
#ifdef HAVE_TESTS
  friend class Test;
#endif
};

#endif // NETWORK_HH
