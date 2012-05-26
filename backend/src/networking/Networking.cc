// Networking.cc
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

#include "Networking.hh"
#include "Util.hh"

#include "workrave.pb.h"

using namespace std;

Networking::Ptr
Networking::create(ICloud::Ptr network, IConfigurator::Ptr configurator)
{
  return Networking::Ptr(new Networking(network, configurator));
}


//! Constructs a cloud
Networking::Networking(ICloud::Ptr network, IConfigurator::Ptr configurator) : network(network), configurator(configurator)
{
  TRACE_ENTER("Networking::Networking");
  configuration_manager = NetworkConfigurationManager::create(network, configurator);
  activity_monitor = NetworkActivityMonitor::create(network);
  TRACE_EXIT();
}


//! Destructs the workrave cloud.
Networking::~Networking()
{
  TRACE_ENTER("Networking::~Networking");
  TRACE_EXIT();
}


//! Initializes the workrave cloud.
void
Networking::init()
{
  TRACE_ENTER("Networking::init");
  configuration_manager->init();
  activity_monitor->init();
  TRACE_EXIT();
}


//! Terminates the network announcer.
void
Networking::terminate()
{
  TRACE_ENTER("Networking::terminate");
  TRACE_EXIT();
}


//! Periodic heartbeart from the core.
void
Networking::heartbeat()
{
  TRACE_ENTER("Networking::heartbeat");
  static bool once = false;
  
  // TODO: debugging code.
  if (!once)
    {
      boost::shared_ptr<workrave::networking::ActivityState> a(new workrave::networking::ActivityState());
      a->set_state(1);
      network->send_message(a, MessageParams::create());
      once = true;
    }
  
  TRACE_EXIT();
}
