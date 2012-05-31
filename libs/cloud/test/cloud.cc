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

#include "cloud/Cloud.hh"
#include "Router.hh"

#include "test.pb.h"

static gboolean on_timer(gpointer data)
{
  TRACE_ENTER("Networking::heartbeat");
  static bool once = false;
  ICloud *network = (ICloud *) data;
  
  // TODO: debugging code.
  if (!once)
    {
      boost::shared_ptr<workrave::cloud::test::ActivityState> a(new workrave::cloud::test::ActivityState());
      a->set_state(1);
      network->send_message(a, MessageParams::create());
      once = true;
    }

  TRACE_EXIT();
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

  g_type_init();
  GMainLoop *loop= g_main_loop_new(NULL, FALSE);

  Router::Ptr network1 = Router::create();
  network1->init(2701, "rob@workrave", "kjsdapkidszahf");

  Router::Ptr network2 = Router::create();
  network2->init(2702, "rob@workrave", "kjsdapkidszahf");
  network2->connect("localhost", 2701);
  
  Router::Ptr network3 = Router::create();
  network3->init(2703, "rob@workrave", "kjsdapkidszahf");
  network3->connect("localhost", 2701);

  Router::Ptr network4 = Router::create();
  network4->init(2704, "rob@workrave", "kjsdapkidszahf");
  network4->connect("localhost", 2703);
  
  g_timeout_add_seconds(2, on_timer, network1.get());
  g_main_loop_run(loop);
  
  return 0;
}
