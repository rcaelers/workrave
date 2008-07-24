// LinkedConfigurationManager.cc --- Linked (networked) ConfigurationManager
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

static const char rcsid[] = "$Id: ConfigurationManager.cc 1087 2006-10-01 18:40:08Z dotsphinx $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>

#include "debug.hh"

#include "LinkedConfigurationManager.hh"
#include "IConfigurator.hh"
#include "CoreFactory.hh"
#include "LinkRouter.hh"
#include "LinkEvent.hh"
#include "ConfigurationLinkEvent.hh"
#include "ResolveConfigurationLinkEvent.hh"
#include "LinkStateLinkEvent.hh"

using namespace std;

LinkedConfigurationManager::LinkedConfigurationManager(LinkRouter *router)
  : router(router), inhibit(false)
{
  core = CoreFactory::get_core();
  config = CoreFactory::get_configurator();
}


LinkedConfigurationManager::~LinkedConfigurationManager()
{
}


//! Initialized the distributed configuration manager
void
LinkedConfigurationManager::init()
{
  router->subscribe("configurationlinkevent", this);
  router->subscribe("linkstatelinkevent", this);

  config->add_listener("", this);

  // FIXME: for testing
  monitor_config("internal/dontchange");
  monitor_config("internal/self-destruct");

  // FIXME:
  monitor_config("timers/micro_pause/activity_sensitive");
  monitor_config("timers/micro_pause/auto_reset");
  monitor_config("timers/micro_pause/limit");
  monitor_config("timers/micro_pause/reset_pred");
  monitor_config("timers/micro_pause/snooze");
  monitor_config("timers/rest_break/activity_sensitive");
  monitor_config("timers/rest_break/auto_reset");
  monitor_config("timers/rest_break/limit");
  monitor_config("timers/rest_break/reset_pred");
  monitor_config("timers/rest_break/snooze");
  monitor_config("timers/daily_limit/activity_sensitive");
  monitor_config("timers/daily_limit/auto_reset");
  monitor_config("timers/daily_limit/limit");
  monitor_config("timers/daily_limit/reset_pred");
  monitor_config("timers/daily_limit/snooze");
  monitor_config("breaks/micro_pause/max_preludes");
  monitor_config("breaks/rest_break/max_preludes");
  monitor_config("breaks/daily_limit/max_preludes");
}


//! Periodic heartbeat
void
LinkedConfigurationManager::heartbeat()
{
}


//! Monitor the specified configurtion key
void
LinkedConfigurationManager::monitor_config(const std::string &key)
{
  watches.push_back(key);
}


//!
void
LinkedConfigurationManager::resolve_config(const std::string &key, const std::string &typed_value)
{
  TRACE_ENTER_MSG("LinkedConfigurationManager::resolve_config", key << " " << typed_value);

  ChangesIter i = needs_resolving.find(key);
  if (i != needs_resolving.end())
    {
      TRACE_MSG("Local user resolved");
      needs_resolving.erase(i);
      resolved[key] = typed_value;

      inhibit = true;
      config->set_typed_value(key, typed_value);
      inhibit = false;
    }

  if (needs_resolving.size() == 0)
    {
      TRACE_MSG("Local user resolved all");

      ConfigurationLinkEvent event;
      for (ChangesIter i = resolved.begin(); i != resolved.end(); i++)
        {
          event.add_change(i->first, i->second);
          TRACE_MSG("add resolved " << i->first << " " << i->second);
        }
      event.set_reason(ConfigurationLinkEvent::RESOLVED);

      router->send_event(&event);
      resolved.clear();

      ResolveConfigurationLinkEvent resolve_event;
      router->send_event_locally(&resolve_event);
    }
  TRACE_EXIT();
}


//! A distributed event was received
void
LinkedConfigurationManager::event_received(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedConfigurationManager::event_received", event->str());

  string eventid = event->get_eventid();

  if (eventid == "configurationlinkevent")
    {
      handle_configuration_event(event);
    }
  else if (eventid == "linkstatelinkevent")
    {
      handle_linkstate_event(event);
    }

  TRACE_EXIT();
}


//! Process incoming link state event
void
LinkedConfigurationManager::handle_linkstate_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedActivityManager::handle_linkstate_event", event->str());

  LinkStateLinkEvent *lse = dynamic_cast<LinkStateLinkEvent *>(event);
  if (lse == NULL)
    {
      return;
    }

  LinkStateLinkEvent::LinkState ls = lse->get_link_state();
  if (ls != LinkStateLinkEvent::LINKSTATE_UP)
    {
      return;
    }

  ConfigurationLinkEvent cle;

  for (ConfigWatchIter i = watches.begin(); i != watches.end(); i++)
    {
      string &key = *i;
      string value;
      bool b = config->get_typed_value(key, value);

      TRACE_MSG("key " << key << " " << value);
      if (b)
        {
          cle.add_change(key, value);
        }
    }
  cle.set_reason(ConfigurationLinkEvent::INITIAL);

  router->send_event(&cle);

  TRACE_EXIT();
}


//! Process incoming configuration event
void
LinkedConfigurationManager::handle_configuration_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedConfigurationManager::handle_configuration_event", event->str());

  ConfigurationLinkEvent *cle = dynamic_cast<ConfigurationLinkEvent *>(event);
  if (cle == NULL)
    {
      return;
    }

  ConfigurationLinkEvent::Reason reason;
  reason = cle->get_reason();

  if (reason == ConfigurationLinkEvent::INITIAL)
    {
      TRACE_MSG("Asking user");
      ask_user(cle);
    }
  else if (reason == ConfigurationLinkEvent::USER ||
           reason == ConfigurationLinkEvent::RESOLVED)
    {
      map<string, string> changes = cle->get_changes();
      map<string, string>::iterator i;

      inhibit = true;
      for (i = changes.begin(); i != changes.end(); i++)
        {
          if (reason == ConfigurationLinkEvent::RESOLVED)
            {
              TRACE_MSG("Remote user resolved:" << i->first);
              needs_resolving.erase(i->first);
            }

          config->set_typed_value(i->first, i->second);
        }
      inhibit = false;
    }

  if (reason == ConfigurationLinkEvent::RESOLVED  &&
      needs_resolving.size() == 0)
    {
      TRACE_MSG("Remote user resolved all");

      ResolveConfigurationLinkEvent resolve_event;
      router->send_event_locally(&resolve_event);
    }

  TRACE_EXIT();
}


//! Process configuration change.
void
LinkedConfigurationManager::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("LinkedConfigurationManager::config_changed_notify", key);
  TRACE_MSG("current_time" << core->get_time());
  
  if (!inhibit)
    {
      ConfigWatchIter i = find(watches.begin(), watches.end(), key);
      if (i != watches.end())
        {
          string value;
          bool b = config->get_typed_value(key, value);
          if (b)
            {
              ConfigurationLinkEvent event;
              
              event.add_change(key, value);
              event.set_reason(ConfigurationLinkEvent::USER);
              router->send_event(&event);
              
              TRACE_MSG("sending " << key << " " << value);
            }
        }
    }
  
  TRACE_EXIT();
}


void
LinkedConfigurationManager::ask_user(ConfigurationLinkEvent *event)
{
  TRACE_ENTER("LinkedConfigurationManager::ask_user");

  map<string, string> changes = event->get_changes();
  map<string, string>::iterator i;

  ResolveConfigurationLinkEvent resolve_event;
  bool added = false;

  for (i = changes.begin(); i != changes.end(); i++)
    {
      string local_value;
      bool b = config->get_typed_value(i->first, local_value);
      TRACE_MSG("Checking " << i->first << " remote : " << i->second << " local: " << local_value);
      if (!b || local_value != i->second)
        {
          TRACE_MSG("Needs resolving");
          resolve_event.add(i->first, i->second);
          needs_resolving[i->first] = i->second;
          added = true;
        }
    }

  if (added)
    {
      TRACE_MSG("Request local resolve");
      router->send_event_locally(&resolve_event);
    }
  TRACE_EXIT();
}
