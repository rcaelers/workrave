// Network.cc
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

#include <iostream>
#include <fstream>

#include "debug.hh"

#include "Network.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "Util.hh"

using namespace std;
using namespace workrave;

//! Constructs a network wrapper
Network::Network()
{
}


//! Destructs the network wrapper
Network::~Network()
{
}


//! Initializes the network wrapper.
void
Network::init()
{
  TRACE_ENTER("Network::init");

  IConfigurator *config = CoreFactory::get_configurator();
  config->set_delay("networking/port", 2);

  // Convert old settings.

  config->set_value("networking/enabled", false, CONFIG_FLAG_DEFAULT);
  config->set_value("networking/port", 2773, CONFIG_FLAG_DEFAULT);
  config->set_value("networking/username", "", CONFIG_FLAG_DEFAULT);
  config->set_value("networking/secret", "", CONFIG_FLAG_DEFAULT);

  config->add_listener("networking", this);

  config->get_value("networking/enabled", enabled);
  config->get_value("networking/port", port);

  if (enabled)
    {
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

  //report_active(false);
  //for (int i = 0; i < BREAK_ID_SIZEOF; i++)
  //  {
  //    report_timer_state(i, false);
  //  }

  TRACE_EXIT();
}


//! Periodic heartbear from the core.
void
Network::heartbeat()
{
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
          // listen(port);
        }
      else
        {
          // stop_listening();
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
          //stop_listening();
          //listen(new_port);
        }
      port = new_port;
    }
}
