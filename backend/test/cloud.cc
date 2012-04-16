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

class CloudTest
{
public:

  void start();

  void on_multicast_data(int size, void *data);
  static gboolean static_on_timer(gpointer data);
  
private:
  IMulticastServer *multicast_server;
  
};

void
CloudTest::start()
{
  multicast_server = new GIOMulticastServer("239.160.181.73", "ff15::1:145", 27273);
  multicast_server->init();
  multicast_server->signal_multicast_data().connect(sigc::mem_fun(*this, &CloudTest::on_multicast_data));

	// UnixNetworkInterfaceMonitor *u = new UnixNetworkInterfaceMonitor();
  // u->init();
  
  g_timeout_add_seconds(10, static_on_timer, this);

}

void
CloudTest::on_multicast_data(int size, void *data)
{
  (void) size;
  printf(">> %s", data);
}


gboolean
CloudTest::static_on_timer(gpointer data)
{
  CloudTest *self = (CloudTest *)data;
  
  self->multicast_server->send("Boe\n", 5);

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

  ;

  g_type_init();
  GMainLoop *loop= g_main_loop_new(NULL, FALSE);

  CloudTest cloud;

  cloud.start();
  
  g_main_loop_run(loop);
  
  return 0;
}
