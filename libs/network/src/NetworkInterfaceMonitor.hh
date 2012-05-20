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

#ifndef NETWORKINTERFACEMONITOR_HH
#define NETWORKINTERFACEMONITOR_HH

#include <map>
#include <string>

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "GIONetworkAddress.hh"

//! 
class NetworkInterfaceMonitor
{
public:
  typedef boost::shared_ptr<NetworkInterfaceMonitor> Ptr;

public:
  virtual ~NetworkInterfaceMonitor() {}

  class NetworkInterfaceInfo
  {
  public:
    NetworkInterfaceInfo() {}
    
    std::string name;
    GIONetworkAddress::Ptr address;
    bool valid;

  private:
    NetworkInterfaceInfo(const NetworkInterfaceInfo &other);
    NetworkInterfaceInfo &operator=(const NetworkInterfaceInfo &);
  };

  static NetworkInterfaceMonitor::Ptr create();
  
  virtual bool init() = 0;
  virtual boost::signals2::signal<void(const NetworkInterfaceInfo &)> &signal_interface_changed() = 0;

private:
};

#endif
