// cloud.cc --- Main
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
#include <fstream>
#include <stdio.h>

#include <X11/Xlib.h>

#include <glib-object.h>
#include <gtk/gtk.h>


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
  XInitThreads();
  
  gtk_init(&argc, &argv);
  
  GMainLoop *loop= g_main_loop_new(NULL, FALSE);

  boost::shared_ptr<boost::barrier> barrier(new boost::barrier(5));
    
  
  Workrave::Ptr workrave1 = Workrave::create(1);
  workrave1->init(barrier);
  
  Workrave::Ptr workrave2 = Workrave::create(2);
  workrave2->init(barrier);

  Workrave::Ptr workrave3 = Workrave::create(3);
  workrave3->init(barrier);

  Workrave::Ptr workrave4 = Workrave::create(4);
  workrave4->init(barrier);

  barrier->wait();
  
  workrave2->connect("localhost", 2701);
  workrave3->connect("localhost", 2701);
  workrave4->connect("localhost", 2703);
  workrave4->connect("localhost", 2701);
  
  g_timeout_add_seconds(1, on_timer, workrave1.get());
  g_timeout_add_seconds(1, on_timer, workrave2.get());
  g_timeout_add_seconds(1, on_timer, workrave3.get());
  g_timeout_add_seconds(1, on_timer, workrave4.get());
  
  g_main_loop_run(loop);
  
  return 0;
}
