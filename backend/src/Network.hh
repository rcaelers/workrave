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


//! Main entry point of all networking functionality.
class Network
  : public INetwork,
    public IConfiguratorListener
{
public:
  Network();
  virtual ~Network();

  // Core internal
  void init();
  void terminate();
  void heartbeat();
  //bool get_remote_active() const;
  //bool is_remote_active(const WRID &remote_id, time_t &since) const;
  //void report_active(bool active);
  //void report_timer_state(int id, bool running);

  void config_changed_notify(const std::string &key);
  void on_enabled_changed();
  void on_port_changed();

private:
  void init_my_id();

private:
  //! My ID
  WRID my_id;

  //! Is the networking enabled?
  bool enabled;

  //! TCP port on which Workrave listens.
  int port;

#ifdef HAVE_TESTS
  friend class Test;
#endif
};

#endif // NETWORK_HH
