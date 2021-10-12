// DistributionManager.hh
//
// Copyright (C) 2002 - 2010 Rob Caelers <robc@krandor.org>
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

#include "config/IConfigurator.hh"
#include "config/IConfiguratorListener.hh"
#include "IDistributionClientMessage.hh"
#include "core/IDistributionManager.hh"

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
  , public workrave::config::IConfiguratorListener
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
  void init(workrave::config::IConfigurator::Ptr conf);
  void heartbeart();
  bool is_master() const override;
  std::string get_master_id() const;
  std::string get_my_id() const;
  int get_number_of_peers() override;
  bool claim();
  bool set_lock_master(bool lock);
  bool connect(std::string url) override;
  bool disconnect(std::string id);
  bool register_client_message(DistributionClientMessageID id, DistributionClientMessageType type, IDistributionClientMessage *callback);
  bool unregister_client_message(DistributionClientMessageID id);

  bool add_listener(DistributionListener *listener);
  bool remove_listener(DistributionListener *listener);

  bool broadcast_client_message(DistributionClientMessageID id, PacketBuffer &buffer);
  bool add_peer(std::string peer) override;
  bool remove_peer(std::string peer) override;
  bool disconnect_all() override;
  bool reconnect_all() override;
  void set_peers(std::string peers, bool connect = true) override;
  std::list<std::string> get_peers() const override;

  // Logging.
  bool add_log_listener(DistributionLogListener *listener) override;
  bool remove_log_listener(DistributionLogListener *listener) override;
  std::list<std::string> get_logs() const override;

  bool get_enabled() const override;
  void set_enabled(bool b) override;

  bool get_listening() const override;
  void set_listening(bool b) override;

  std::string get_username() const override;
  void set_username(std::string name) override;

  std::string get_password() const override;
  void set_password(std::string name) override;

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
  void sanitize_peer(std::string &peer);
  void parse_peers(std::string peers, bool connect = true);
  void write_peers();
  void read_configuration();
  void config_changed_notify(const std::string &key) override;

  void fire_log_event(std::string message);
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
  workrave::config::IConfigurator::Ptr configurator{nullptr};

  //! Link to other clients
  DistributionLink *link{nullptr};

  //! Current State
  NodeState state{NODE_ACTIVE};

  //! All peers
  std::list<std::string> peer_urls;

  // ! All log messages
  std::list<std::string> log_messages;

  //! Log listeners.
  LogListeners log_listeners;

  //! Event listeners.
  Listeners listeners;

  //! Current master.
  std::string current_master;
};

#endif // DISTRIBUTIOMANAGER_HH
