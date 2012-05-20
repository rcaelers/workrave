// NetworkConfigurationManager.cc --- Network (networked) ConfigurationManager
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>

#include "debug.hh"

#include "TimeSource.hh"
#include "NetworkConfigurationManager.hh"
#include "IConfigurator.hh"
#include "CoreFactory.hh"

#include "cloud.pb.h"

using namespace std;
using namespace workrave::utils;

NetworkConfigurationManager::Ptr
NetworkConfigurationManager::create(INetwork::Ptr network, IConfigurator::Ptr configurator)
{
  return NetworkConfigurationManager::Ptr(new NetworkConfigurationManager(network, configurator));
}


NetworkConfigurationManager::NetworkConfigurationManager(INetwork::Ptr network, IConfigurator::Ptr configurator)
  : network(network), configurator(configurator)
{
}


NetworkConfigurationManager::~NetworkConfigurationManager()
{
}


//! Initialized the distributed configuration manager
void
NetworkConfigurationManager::init()
{
  network->signal_message(1, cloud::Configuration::kTypeFieldNumber)
    .connect(boost::bind(&NetworkConfigurationManager::on_configuration_message, this, _1));

  configurator->add_listener("", this);

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


//! Monitor the specified configurtion key
void
NetworkConfigurationManager::monitor_config(const std::string &key)
{
  watches.push_back(key);
}


void
NetworkConfigurationManager::on_configuration_message(NetworkMessageBase::Ptr message)
{
  TRACE_ENTER("NetworkConfigurationManager::on_configuration_message");
  // NetworkMessage<cloud::ActivityState>::Ptr as = boost::dynamic_pointer_cast<NetworkMessage<cloud::ActivityState> >(message);
  
  boost::shared_ptr<cloud::Configuration> a = message->as<cloud::Configuration>();

  if (a)
    {
      cloud::Configuration_Reason reason = a->reason();

      if (reason == cloud::Configuration_Reason_INITIAL)
        {
          TRACE_MSG("Asking user");
          // TODO:
        }
      else if (reason == cloud::Configuration_Reason_USER)
        {
          google::protobuf::RepeatedPtrField<cloud::Configuration_Setting> changes = a->changes();

          for (google::protobuf::RepeatedPtrField<cloud::Configuration_Setting>::iterator i = changes.begin(); i != changes.end(); i++)
            {
              configurator->set_typed_value(i->key(), i->value());
            }
        }
    }
  TRACE_EXIT();
}


//! Send initial configuration
void
NetworkConfigurationManager::send_initial()
{
  TRACE_ENTER("NetworkConfigurationManager::send_initial");
  NetworkMessage<cloud::Configuration>::Ptr m = NetworkMessage<cloud::Configuration>::create();
  m->msg()->set_reason(cloud::Configuration_Reason_INITIAL);

  for (ConfigWatchIter i = watches.begin(); i != watches.end(); i++)
    {
      string &key = *i;
      string value;
      bool b = configurator->get_typed_value(key, value);

      TRACE_MSG("key " << key << " " << value);
      if (b)
        {
          cloud::Configuration_Setting *c = m->msg()->add_changes();
          c->set_key(key);
          c->set_value(value);
        }
    }
  
  network->send_message(m);

  TRACE_EXIT();
}


//! Process configuration change.
void
NetworkConfigurationManager::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("NetworkConfigurationManager::config_changed_notify", key);
  TRACE_MSG("current_time" << TimeSource::get_time());
  
  ConfigWatchIter i = find(watches.begin(), watches.end(), key);
  if (i != watches.end())
    {
      string value;
      bool b = configurator->get_typed_value(key, value);
      if (b)
        {
          NetworkMessage<cloud::Configuration>::Ptr m = NetworkMessage<cloud::Configuration>::create();
          m->msg()->set_reason(cloud::Configuration_Reason_USER);

          cloud::Configuration_Setting *c = m->msg()->add_changes();
          c->set_key(key);
          c->set_value(value);
                           
          network->send_message(m);
          TRACE_MSG("sending " << key << " " << value);
        }
    }
  
  TRACE_EXIT();
}


