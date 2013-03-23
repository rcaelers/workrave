// cloud.cc --- Main
//
// Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
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

#include "CloudControl.hh"

static gboolean on_timer(gpointer data)
{
  TRACE_ENTER("Networking::heartbeat");
  static bool once = false;

  if (!once)
    {
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

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  CloudControl::Ptr cc = CloudControl::create();
  cc->init();
  
  g_timeout_add_seconds(1, on_timer, NULL);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);
  
  return 0;
}
