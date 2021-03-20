// DistributionManager.hh
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008, 2009, 2010 Rob Caelers <robc@krandor.org>
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

#ifndef DISTRIBUTIOMANAGER_HH
#define DISTRIBUTIOMANAGER_HH

#include <string>
#include <list>

using namespace std;

#include "IConfiguratorListener.hh"
#include "IDistributionClientMessage.hh"
#include "IDistributionManager.hh"

using namespace workrave;
namespace workrave
{
  class IDistributionManager;
  class DistributionLogListener;
} // namespace workrave

class DistributionLink;
class Configurator;
class DistributionListener;
class PacketBuffer;

class DistributionManager
  : public IDistributionManager
  , public IConfiguratorListener
{
public:
  enum NodeState
  {
    NODE_ACTIVE,
    NODE_PASSIVE,
    NODE_STANDBY
  };

  DistributionManager() = default;
  ~DistributionManager() override;

  NodeState get_state() const;
  void init(Configurator *conf);
  void heartbeart();
  bool is_master() const override;
  string get_master_id() const;
  string get_my_id() const;
  int get_number_of_peers() override;
  bool claim();
  bool set_lock_master(bool lock);
  bool connect(string url) override;
  bool disconnect(string id);
  bool register_client_message(DistributionClientMessageID id,
                               DistributionClientMessageType type,
                               IDistributionClientMessage *callback);
  bool unregister_client_message(DistributionClientMessageID id);

  bool add_listener(DistributionListener *listener);
  bool remove_listener(DistributionListener *listener);

  bool broadcast_client_message(DistributionClientMessageID id, PacketBuffer &buffer);
  bool add_peer(string peer) override;
  bool remove_peer(string peer) override;
  bool disconnect_all() override;
  bool reconnect_all() override;
  void set_peers(string peers, bool connect = true) override;
  list<string> get_peers() const override;

  // Logging.
  bool add_log_listener(DistributionLogListener *listener) override;
  bool remove_log_listener(DistributionLogListener *listener) override;
  list<string> get_logs() const override;

  bool get_enabled() const override;
  void set_enabled(bool b) override;

  bool get_listening() const override;
  void set_listening(bool b) override;

  string get_username() const override;
  void set_username(string name) override;

  string get_password() const override;
  void set_password(string name) override;

  int get_port() const override;
  void set_port(int v) override;

  int get_reconnect_attempts() const override;
  void set_reconnect_attempts(int v) override;

  int get_reconnect_interval() const override;
  void set_reconnect_interval(int v) override;

  // DistributionLinkListener
  void master_changed(bool result, std::string id);
  void signon_remote_client(char *client_id);
  void signoff_remote_client(char *client_id);
  void log(const char *fmt, ...);

private:
  void sanitize_peer(string &peer);
  void parse_peers(string peers, bool connect = true);
  void write_peers();
  void read_configuration();
  void config_changed_notify(const string &key) override;

  void fire_log_event(string message);
  void fire_signon_client(char *id);
  void fire_signoff_client(char *id);

private:
  typedef std::list<DistributionLogListener *> LogListeners;
  typedef std::list<DistributionLogListener *>::iterator LogListenerIter;

  typedef std::list<DistributionListener *> Listeners;
  typedef std::list<DistributionListener *>::iterator ListenerIter;

  //! Is distribution operation enabled?
  bool network_enabled{false};
  bool server_enabled{false};

  //! Access to the configuration.
  Configurator *configurator{nullptr};

  //! Link to other clients
  DistributionLink *link{nullptr};

  //! Current State
  NodeState state{NODE_ACTIVE};

  //! All peers
  list<string> peer_urls;

  // ! All log messages
  list<string> log_messages;

  //! Log listeners.
  LogListeners log_listeners;

  //! Event listeners.
  Listeners listeners;

  //! Current master.
  string current_master;
};

#endif // DISTRIBUTIOMANAGER_HH
