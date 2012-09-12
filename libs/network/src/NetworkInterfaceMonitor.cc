// NetworkInterfaceMonitor.cc
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

#include "debug.hh"
#include "NetworkInterfaceMonitor.hh"

#if defined(PLATFORM_OS_UNIX)

#if defined(HAVE_NETLINK)
#include "NetlinkNetworkInterfaceMonitor.hh"
#endif

#include "UnixNetworkInterfaceMonitor.hh"
#endif


NetworkInterfaceMonitor::Ptr
NetworkInterfaceMonitor::create()
{
  NetworkInterfaceMonitor::Ptr ret;
  
#if defined(PLATFORM_OS_UNIX)
#  if defined(HAVE_NETLINK)
  ret = NetworkInterfaceMonitor::Ptr(new NetlinkNetworkInterfaceMonitor());
#  else
  ret = NetworkInterfaceMonitor::Ptr(new UnixNetworkInterfaceMonitor());
#endif
#endif

  return ret;
}

