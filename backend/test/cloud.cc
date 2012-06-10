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

#include "Workrave.hh"

#include <boost/signals2.hpp>

static gboolean on_timer(gpointer data)
{
  Workrave *w = (Workrave *)data;
  
  w->heartbeat();

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

  Workrave::Ptr workrave1 = Workrave::create();
  
  Workrave::Ptr workrave2 = Workrave::create();
  workrave2->connect("localhost", 2701);

  Workrave::Ptr workrave3 =  Workrave::create();
  workrave3->connect("localhost", 2701);

  Workrave::Ptr workrave4 =  Workrave::create();
  workrave4->connect("localhost", 2703);
  workrave4->connect("localhost", 2701);
  
  g_timeout_add_seconds(1, on_timer, workrave1.get());
  g_timeout_add_seconds(1, on_timer, workrave2.get());
  g_timeout_add_seconds(1, on_timer, workrave3.get());
  g_timeout_add_seconds(1, on_timer, workrave4.get());
  
  g_main_loop_run(loop);
  
  return 0;
}
