// Networking.hh --- Networking network server
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

#ifndef NETWORKING_HH
#define NETWORKING_HH

#include <list>
#include <map>
#include <string>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "IConfigurator.hh"

#include "Cloud.hh"
#include "NetworkConfigurationManager.hh"
#include "NetworkActivityMonitor.hh"

using namespace workrave;
using namespace workrave::config;

class Networking
{
public:
  typedef boost::shared_ptr<Networking> Ptr;
  
public:
  static Ptr create(ICloud::Ptr network, IConfigurator::Ptr configurator);

public:  
  Networking(ICloud::Ptr network, IConfigurator::Ptr configurator);
  virtual ~Networking();

  void init();
  void terminate();
  void heartbeat();

private:
  //! 
  ICloud::Ptr network;

  //!
  IConfigurator::Ptr configurator;
  
  //! 
  NetworkConfigurationManager::Ptr configuration_manager;

  //!
  NetworkActivityMonitor::Ptr activity_monitor;
};


#endif // NETWORKING_HH
