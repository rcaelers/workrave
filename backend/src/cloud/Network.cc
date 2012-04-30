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
#include <sstream>

#include "debug.hh"

#include "Network.hh"
#include "NetworkRouter.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "Util.hh"

using namespace std;
using namespace workrave;

//! Constructs a network wrapper
Network::Network() : router(NULL)
{
}


//! Destructs the network wrapper
Network::~Network()
{
  delete router;
}


//! Initializes the network wrapper.
void
Network::init(int port)
{
  TRACE_ENTER("Network::init");

  this->port = port;
  init_my_id();

  router = new NetworkRouter(my_id);
  
  router->init(port);
  
  TRACE_EXIT();
}

//! Connects to a remote workrave.
void
Network::connect(string host, int port)
{
  return router->connect(host, port);
}

//! Terminates the networking
void
Network::terminate()
{
  TRACE_ENTER("Network::terminate");
  TRACE_EXIT();
}

//! Periodic heartbear from the core.
void
Network::heartbeat()
{
  router->heartbeat();
}

//! Initializes the network wrapper.
void
Network::init_my_id()
{
  TRACE_ENTER("Network::init_my_id");
  bool ok = false;
  stringstream ss;

  ss << Util::get_home_directory() << "id-" << port << ends;
  string idfilename = ss.str();

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

