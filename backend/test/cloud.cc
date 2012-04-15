// cloud.cc --- Main
//
// Copyright (C) 2012 Rob Caelers & Raymond Penners
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
#include <fstream>
#include <stdio.h>

#include <glib-object.h>

#include "GIOMulticastServer.hh"
#include "UnixNetworkInterfaceMonitor.hh"
#include "NetlinkNetworkInterfaceMonitor.hh"

static IMulticastServer *multicast_server;

static gboolean
timer(gpointer data)
{
  (void) data;
  
  multicast_server->send("Boe\n", 5);

  return G_SOURCE_CONTINUE;
}

int
main(int argc, char **argv)
{
  (void) argc;
  (void) argv;
  
#ifdef TRACING
  Debug::init();
#endif

  GMainLoop *loop;

  g_type_init();

  loop = g_main_loop_new(NULL, FALSE);

	// UnixNetworkInterfaceMonitor *u = new UnixNetworkInterfaceMonitor();
  // u->init();
    
  multicast_server = new GIOMulticastServer("239.160.181.73", "ff15::1:145", 27273);
  multicast_server->init();

  g_timeout_add_seconds(10, timer, NULL);
  g_main_loop_run (loop);
  
  return 0;
}
