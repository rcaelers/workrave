// NetworkConfigurationManager.hh
//
// Copyright (C) 2007, 2008, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef NETWORKCONFIGURATIONMANAGER_HH
#define NETWORKCONFIGURATIONMANAGER_HH

#include <list>
#include <map>

#include "cloud/Cloud.hh"
#include "config/Config.hh" 

using namespace workrave;
using namespace workrave::cloud;

// Forward declarion of external interface.
namespace workrave {
  class ICore;
  class LinkEvent;
}

class ConfigurationLinkEvent;
class LinkRouter;

using namespace workrave::config;

class NetworkConfigurationManager
  : public IConfiguratorListener
{
public:
  typedef boost::shared_ptr<NetworkConfigurationManager> Ptr;

public:
  static Ptr create(ICloud::Ptr network, IConfigurator::Ptr configurator);

public:
  NetworkConfigurationManager(ICloud::Ptr network, IConfigurator::Ptr configurator);
  virtual ~NetworkConfigurationManager();

  // from ICloud
  void monitor_config(const std::string &key);
  
  //! Initializes the monitor
  void init();

  //! IConfiguratorListener
  void config_changed_notify(const std::string &key);

  void send_initial();
  
private:
  typedef std::list<std::string> ConfigWatches;
  typedef ConfigWatches::iterator ConfigWatchIter;

  typedef std::map<std::string, std::string> Changes;
  typedef Changes::iterator ChangesIter;

private:
  void on_configuration_message(Message::Ptr, MessageContext::Ptr);
  
private:
  //! The networking core
  ICloud::Ptr network;

  //! The main configurator.
  IConfigurator::Ptr configurator;

  //! All configuration keys that must be watched.
  ConfigWatches watches;
};

#endif // NETWORKCONFIGURATIONMANAGER_HH
