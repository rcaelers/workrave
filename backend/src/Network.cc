// Network.cc
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <fstream>

#include "debug.hh"

#include "Network.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "LinkRouter.hh"
#include "LinkedActivityMonitor.hh"
#include "LinkedHistoryManager.hh"
#include "LinkedConfigurationManager.hh"

#include "Util.hh"

using namespace std;
using namespace workrave;

//! Constructs a network wrapper
Network::Network()
{
  router = new LinkRouter(my_id);
  linked_activity = new LinkedActivityMonitor;
  linked_history = new LinkedHistoryManager(router);
  linked_config = new LinkedConfigurationManager(router);
}


//! Destructs the network wrapper
Network::~Network()
{
  delete linked_config;
  delete linked_history;
  delete linked_activity;
  delete router;
}


//! Initializes the network wrapper.
void
Network::init()
{
  TRACE_ENTER("Network::init");

  IConfigurator *config = CoreFactory::get_configurator();
  config->set_delay("networking/port", 2);

  // Convert old settings.

  config->rename_key("distribution/port",       "networking/port");
  config->rename_key("distribution/enabled",    "networking/enabled");
  config->rename_key("distribution/username",   "networking/username");
  config->rename_key("distribution/peers",      "networking/peers");

  config->remove_key("distribution/password");
  config->remove_key("distribution/reconnect_attempts");
  config->remove_key("distribution/reconnect_interval");

  // Set defaults.

  config->set_value("networking/enabled", false, CONFIG_FLAG_DEFAULT);
  config->set_value("networking/port", 2773, CONFIG_FLAG_DEFAULT);
  config->set_value("networking/username", "", CONFIG_FLAG_DEFAULT);
  config->set_value("networking/secret", "", CONFIG_FLAG_DEFAULT);
  config->set_value("networking/peers", "", CONFIG_FLAG_DEFAULT);

  router->init();
  linked_activity->init();
  linked_history->init();
  linked_config->init();

  config->add_listener("networking", this);

  config->get_value("networking/enabled", enabled);
  config->get_value("networking/port", port);

  if (enabled)
    {
      listen(port);
    }

  init_my_id();
  
  TRACE_EXIT();
}

//! Initializes the network wrapper.
void
Network::init_my_id()
{
  TRACE_ENTER("Network::init_my_id");
  bool ok = false;
  string idfilename = Util::get_home_directory() + "id";

  if (Util::file_exists(idfilename))
    {
      ifstream file(idfilename.c_str());
      
      if (file)
        {
          string id_str;
          file >> id_str;

          if (id_str.length() == WRID::STR_LENGTH)
            {
              ok = my_id.set(id_str);              
            }
          file.close();
        }
    }

  if (! ok)
    {
      ofstream file(idfilename.c_str());

      file << my_id.str() << endl;
      file.close();
    }


  TRACE_EXIT();
}



//! Terminates the networking
void
Network::terminate()
{
  TRACE_ENTER("Network::terminate");

  report_active(false);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      report_timer_state(i, false);
    }

  linked_history->terminate();
  TRACE_EXIT();
}


//! Periodic heartbear from the core.
void
Network::heartbeat()
{
  linked_config->heartbeat();
  linked_history->heartbeat();
}


//! Connect to a remote workrave and return the link-id
void
Network::connect(string url)
{
  return router->connect(url);
}


//! Disconnect the all links
void
Network::leave()
{
  router->disconnect_all();
}


//! Start listen at a certain TCP port.
bool
Network::listen(int port)
{
  return router->listen(port);
}


//! Start listen at a certain TCP port.
void
Network::stop_listening()
{
  return router->stop_listening();
}


//! Connect to a remote workrave and return the link-id
bool
Network::connect(const string &host, int port, string &link_id)
{
  return router->connect(host, port, link_id);
}


//! Disconnect the specified link-id
bool
Network::disconnect(const string &link_id)
{
  return router->disconnect(link_id);
}


//! Send an event to all workrave clients.
bool
Network::send_event(LinkEvent *event)
{
  return router->send_event(event);
}


//! Send an event through a single network link
bool
Network::send_event_to_link(const WRID &link_id, LinkEvent *event)
{
  return router->send_event_to_link(link_id, event);
}


//! Subscribe to the specified network event.
bool
Network::subscribe(const std::string &eventid, ILinkEventListener *listener)
{
  return router->subscribe(eventid, listener);
}


//! Unsubscribe from the specified network event.
bool
Network::unsubscribe(const std::string &eventid, ILinkEventListener *listener)
{
  return router->unsubscribe(eventid, listener);
}


//! Register a configuration key that MUST the the same on all Workrave clients.
void
Network::monitor_config(const std::string &key)
{
  linked_config->monitor_config(key);
}


//! Resolve a configuration setting when multiple client have a different value.
void
Network::resolve_config(const std::string &key, const std::string &typed_value)
{
  linked_config->resolve_config(key, typed_value);
}


//! Is a remote workrave client active?
bool
Network::get_remote_active() const
{
  return linked_activity->get_active();
}


//! Is a remote workrave client active?
bool
Network::is_remote_active(const WRID &remote_id, time_t &since) const
{
  return linked_activity->is_active(remote_id, since);
}


//! Informs remote clients about local activity
void
Network::report_active(bool active)
{
  linked_history->report_user_activity(active);
}


//! Informs remote clients about local break state
void
Network::report_timer_state(int id, bool active)
{
  linked_history->report_timer_activity(id, active);
}


//! Networking configuration changed.
void
Network::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("Network::config_changed_notify", key);

  if (key == "networking/port")
    {
      on_port_changed();
    }
  else if (key == "networking/enabled")
    {
      on_enabled_changed();
    }
  TRACE_EXIT();
}


//! The networking enabled configuration changed.
void
Network::on_enabled_changed()
{
  bool new_enabled;
  IConfigurator *config = CoreFactory::get_configurator();
  bool ok = config->get_value("networking/enabled", new_enabled);
  if (ok && enabled != new_enabled)
    {
      if (new_enabled)
        {
          listen(port);
        }
      else
        {
          stop_listening();
        }

      enabled = new_enabled;
    }
}


//! The listen port configuration changed.
void
Network::on_port_changed()
{
  if (enabled)
    {
      int new_port = 0;
      IConfigurator *config = CoreFactory::get_configurator();
      bool ok = config->get_value("networking/port", new_port);
      if (ok && port != new_port)
        {
          stop_listening();
          listen(new_port);
        }
      port = new_port;
    }
}
