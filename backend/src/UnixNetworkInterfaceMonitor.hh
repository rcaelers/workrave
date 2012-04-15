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

#ifndef UNIXNETWORKINTERFACEMONITOR_HH
#define UNIXNETWORKINTERFACEMONITOR_HH

#include <map>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "NetworkInterfaceMonitorBase.hh"
#include "SocketDriver.hh"

using namespace workrave;

//! Listen socket implementation using GIO
class UnixNetworkInterfaceMonitor : public NetworkInterfaceMonitorBase
{
public:
  UnixNetworkInterfaceMonitor();
  virtual ~UnixNetworkInterfaceMonitor();

  virtual bool init();

private:
  class NetworkAddress
  {
  public:
    NetworkAddress() : address(NULL) {}
    ~NetworkAddress()
    {
      g_object_unref(address);
    }
    
    bool operator==(const NetworkAddress &other)
    {
      return g_inet_address_equal(address, other.address);
    }
    
    GInetAddress *address;
    
  };

  class NetworkHardware
  {
  public:
    NetworkHardware() : valid(false) {}
    
    std::string name;
    std::list<NetworkAddress> addresses;
    bool valid;
  };

  typedef std::map<guint, NetworkHardware> NetworkHardwareList;
  typedef NetworkHardwareList::const_iterator NetworkHardwareCIter;

private:
  
private:
  NetworkHardwareList interfaces;
};

#endif
