// UnixNetworkInterfaceMonitor.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "debug.hh"

#include "UnixNetworkInterfaceMonitor.hh"
#include "GIONetworkAddress.hh"

using namespace std;


//! Creates a netlink network interface monitor.
UnixNetworkInterfaceMonitor::UnixNetworkInterfaceMonitor()
{
}


//! Destructs the netlink network interface monitor.
UnixNetworkInterfaceMonitor::~UnixNetworkInterfaceMonitor()
{
}


bool
UnixNetworkInterfaceMonitor::init()
{
  struct ifaddrs *ifa_list;

  if (getifaddrs(&ifa_list) != 0)
    {
      g_warning ("Failed to retrieve list of network interfaces:%s\n", strerror (errno));
      return false;
    }

  for (struct ifaddrs *ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next)
    {
      if (!(ifa->ifa_flags & IFF_POINTOPOINT) &&
           (ifa->ifa_flags & IFF_UP))
        {
          // g_debug("ifaddr %s ", ifa->ifa_name);
        }
    }
  freeifaddrs(ifa_list);
  return true;
}
