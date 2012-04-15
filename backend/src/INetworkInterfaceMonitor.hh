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

#ifndef INETWORKINTERFACEMONITOR_HH
#define INETWORKINTERFACEMONITOR_HH

#include <map>
#include <string>
#include <sigc++/sigc++.h>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

//! 
class INetworkInterfaceMonitor
{
public:
  virtual ~INetworkInterfaceMonitor() {}

  class NetworkInterfaceChange
  {
  public:
    NetworkInterfaceChange() {}
    
    std::string name;
    GInetAddress *address;
    bool valid;

  private:
    NetworkInterfaceChange(const NetworkInterfaceChange &other);
    NetworkInterfaceChange &operator=(const NetworkInterfaceChange &);
  };
  
  virtual bool init() = 0;
  virtual sigc::signal<void,  const NetworkInterfaceChange &> &signal_interface_changed() = 0;

private:
};

#endif
