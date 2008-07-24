// LinkedConfigurationManager.hh
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
// $Id: LinkedConfigurationManager.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef LINKEDCONFIGURATIONMANAGER_HH
#define LINKEDCONFIGURATIONMANAGER_HH

#include <list>
#include <map>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "ILinkEventListener.hh"
#include "IConfiguratorListener.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave {
  class ICore;
  class IConfigurator;
  class LinkEvent;
}

class ConfigurationLinkEvent;
class LinkRouter;


class LinkedConfigurationManager
  : public IConfiguratorListener,
    public ILinkEventListener
{
private:
  typedef std::list<std::string> ConfigWatches;
  typedef ConfigWatches::iterator ConfigWatchIter;

  typedef std::map<std::string, std::string> Changes;
  typedef Changes::iterator ChangesIter;

public:
  LinkedConfigurationManager(LinkRouter *router);
  virtual ~LinkedConfigurationManager();

  // from INetwork
  void monitor_config(const std::string &key);
  void resolve_config(const std::string &key, const std::string &typed_value);

  //! Initializes the monitor
  void init();

  //! Periodic heartbeat from core
  void heartbeat();

  // ILinkEventListener
  void event_received(LinkEvent *event);

  //! IConfiguratorListener
  void config_changed_notify(const std::string &key);

private:
  void handle_configuration_event(LinkEvent *event);
  void handle_linkstate_event(LinkEvent *event);
  void ask_user(ConfigurationLinkEvent *event);

private:
  //! The main Core
  ICore *core;

  //! The main configurator.
  IConfigurator *config;

  //! Distributed event router
  LinkRouter *router;

  //! All configuration keys that must be watched.
  ConfigWatches watches;

  //! Ignore changes while we are making changes ourselves
  bool inhibit;

  //! Configuration changes that needs to be resolved
  Changes needs_resolving;

  //! Configuration changes that needs to be resolved
  Changes resolved;
};

#endif // LINKEDCONFIGURATIONMANAGER_HH
