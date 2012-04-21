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
#include "NetworkAnnounce.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

#include "Util.hh"

using namespace std;
using namespace workrave;

//! Constructs a network wrapper
Network::Network() 
{
  init_my_id();

  announcer = new NetworkAnnounce(my_id);
}


//! Destructs the network wrapper
Network::~Network()
{
  delete announcer;
}


//! Initializes the network wrapper.
void
Network::init()
{
  TRACE_ENTER("Network::init");

  announcer->init();
  
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
  TRACE_EXIT();
}


//! Periodic heartbear from the core.
void
Network::heartbeat()
{
  announcer->heartbeat();
}
