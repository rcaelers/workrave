// Cloud.hh --- Networking network server
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

#ifndef CLOUD_HH
#define CLOUD_HH

#include <list>
#include <map>
#include <string>

#include <sigc++/sigc++.h>

#include <boost/shared_ptr.hpp>

#include "IConfigurator.hh"

#include "INetwork.hh"
#include "NetworkConfigurationManager.hh"
#include "NetworkActivityMonitor.hh"

using namespace workrave;
using namespace workrave::network;
using namespace workrave::config;

class Cloud
{
public:
  typedef boost::shared_ptr<Cloud> Ptr;
  
public:
  static Ptr create(INetwork::Ptr network, IConfigurator::Ptr configurator);

public:  
  Cloud(INetwork::Ptr network, IConfigurator::Ptr configurator);
  virtual ~Cloud();

  void init();
  void terminate();
  void heartbeat();

private:
  //! 
  INetwork::Ptr network;

  //!
  IConfigurator::Ptr configurator;
  
  //! 
  NetworkConfigurationManager::Ptr configuration_manager;

  //!
  NetworkActivityMonitor::Ptr activity_monitor;
};


#endif // CLOUD_HH
