// Network.hh --- Networking link server
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#include "UUID.hh"

// Forward declarion of external interfaces.
namespace workrave
{
  class UUID;
  class ICore;
}
using namespace workrave;


// Forward declarion of internal interfaces.
class LinkRouter;
class LinkedActivityMonitor;
class LinkedHistoryManager;
class LinkedConfigurationManager;


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
  bool listen(int port);
  void stop_listening();
  bool connect(const std::string &host, int port, std::string &link_id);
  bool disconnect(const std::string &link_id);
  void heartbeat();
  bool get_remote_active() const;
  bool is_remote_active(const UUID &remote_id, time_t &since) const;
  void report_active(bool active);
  void report_timer_state(int id, bool running);

  // INetwork
  virtual void connect(std::string url);
  virtual void leave();
  virtual bool send_event(LinkEvent *event);
  virtual bool send_event_to_link(const workrave::UUID &link_id, LinkEvent *event);
  virtual bool subscribe(const std::string &eventid, ILinkEventListener *listener);
  virtual bool unsubscribe(const std::string &eventid, ILinkEventListener *listener);
  virtual void monitor_config(const std::string &key);
  virtual void resolve_config(const std::string &key, const std::string &typed_value);

  void config_changed_notify(const std::string &key);
  void on_enabled_changed();
  void on_port_changed();

private:
  void fire_event(LinkEvent *event);
  void init_my_id();

private:
  //! My ID
  UUID my_id;

  //! Distributed event router
  LinkRouter *router;

  //! Distributed state monitoring.
  LinkedActivityMonitor *linked_activity;

  //! Distributed history.
  LinkedHistoryManager *linked_history;

  //! Distributed configuration management;
  LinkedConfigurationManager *linked_config;

  //! Is the networking enabled?
  bool enabled;

  //! TCP port on which Workrave listens.
  int port;

#ifdef HAVE_TESTS
  friend class Test;
#endif
};

#endif // NETWORK_HH
