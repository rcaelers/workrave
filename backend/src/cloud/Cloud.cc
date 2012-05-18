// Cloud.cc
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
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <glib.h>

#include "debug.hh"

#include "Cloud.hh"
#include "Util.hh"

#include "cloud.pb.h"

using namespace std;

Cloud::Ptr
Cloud::create(INetwork::Ptr network, IConfigurator::Ptr configurator)
{
  return Cloud::Ptr(new Cloud(network, configurator));
}


//! Constructs a cloud
Cloud::Cloud(INetwork::Ptr network, IConfigurator::Ptr configurator) : network(network), configurator(configurator)
{
  TRACE_ENTER("Cloud::Cloud");
  configuration_manager = NetworkConfigurationManager::create(network, configurator);
  activity_monitor = NetworkActivityMonitor::create(network);
  TRACE_EXIT();
}


//! Destructs the workrave cloud.
Cloud::~Cloud()
{
  TRACE_ENTER("Cloud::~Cloud");
  TRACE_EXIT();
}


//! Initializes the workrave cloud.
void
Cloud::init()
{
  TRACE_ENTER("Cloud::init");
  configuration_manager->init();
  activity_monitor->init();
  TRACE_EXIT();
}


//! Terminates the network announcer.
void
Cloud::terminate()
{
  TRACE_ENTER("Cloud::terminate");
  TRACE_EXIT();
}


//! Periodic heartbeart from the core.
void
Cloud::heartbeat()
{
  TRACE_ENTER("Cloud::heartbeat");
  static bool once = false;
  
  // TODO: debugging code.
  if (!once)
    {
      NetworkMessage<cloud::ActivityState>::Ptr as = NetworkMessage<cloud::ActivityState>::create();
      as->scope = NetworkClient::SCOPE_DIRECT;
      as->authenticated = true;

      boost::shared_ptr<cloud::ActivityState> a = as->msg();
      as->msg()->set_state(1);
      network->send_message(as);

      once = true;
    }
  
  TRACE_EXIT();
}
