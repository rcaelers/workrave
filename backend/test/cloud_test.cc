// cloud_test.cc
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

#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "debug.hh"
#include "Workrave.hh"

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/signals2.hpp>

using namespace std;

struct Fixture
{
  Fixture()
  {
    BOOST_TEST_MESSAGE("global setup fixture");

#ifdef TRACING
    Debug::init();
#endif

    g_type_init();
    XInitThreads();
    
    gtk_init(0, 0);

    boost::shared_ptr<boost::barrier> barrier(new boost::barrier(num_workraves + 1));

    for (int i = 0; i < num_workraves; i++)
      {
        workraves[i] = Workrave::create(i);
        workraves[i]->init(barrier);
      }
    barrier->wait();
    
    // boost::function<void ()> f3 = [](){ std::cout<<"f3()"<<std::endl; };

    workraves[1]->invoke(boost::bind(&Workrave::connect, workraves[1], "localhost", 2701));
    workraves[2]->invoke(boost::bind(&Workrave::connect, workraves[2], "localhost", 2701));
    workraves[3]->invoke(boost::bind(&Workrave::connect, workraves[3], "localhost", 2703));
    workraves[3]->invoke(boost::bind(&Workrave::connect, workraves[3], "localhost", 2701));

    sleep(5);
    
    for (int i = 0; i < num_workraves; i++)
      {
        cores[i] = workraves[i]->get_core();
      }
  }
  
  ~Fixture()
  {
    BOOST_TEST_MESSAGE("global teardown fixture");
  }

  //GMainLoop *loop;
  const static int num_workraves = 4;
  Workrave::Ptr workraves[num_workraves];
  ICore::Ptr cores[num_workraves];
};

BOOST_FIXTURE_TEST_SUITE(s, Fixture)

BOOST_AUTO_TEST_CASE(test_case1)
{
  OperationMode mode = cores[0]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);

  mode = cores[1]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);

  mode = cores[2]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);

  mode = cores[3]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);

  OperationMode m = workraves[0]->invoke_sync(boost::bind(&ICore::get_operation_mode, cores[0]));
  
  workraves[0]->invoke_sync(boost::bind(&ICore::set_operation_mode, cores[0], OPERATION_MODE_SUSPENDED));

  sleep(5);

  mode = cores[0]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_SUSPENDED);

  mode = cores[1]->get_operation_mode();
  BOOST_CHECK_EQUAL(mode, OPERATION_MODE_SUSPENDED);

  // mode = cores[2]->get_operation_mode();
  // BOOST_CHECK_EQUAL(mode, OPERATION_MODE_SUSPENDED);

  // mode = cores[3]->get_operation_mode();
  // BOOST_CHECK_EQUAL(mode, OPERATION_MODE_SUSPENDED);
}

// BOOST_AUTO_TEST_CASE(test_case2)
// {
//   //    BOOST_CHECK_EQUAL( i, 0 );
// }

BOOST_AUTO_TEST_SUITE_END()



// #include <fstream>
// #include <stdio.h>






// int
// main(int argc, char **argv)
// {
//   (void) argc;
//   (void) argv;
  
// #ifdef TRACING
//   Debug::init();
// #endif

//   g_type_init();

//   GMainLoop *loop= g_main_loop_new(NULL, FALSE);

//   Workrave::Ptr workrave[0] = Workrave::create();
  
//   Workrave::Ptr workrave[1] = Workrave::create();
//   workrave[1]->connect("localhost", 2701);

//   Workrave::Ptr workrave[2] = Workrave::create();
//   workrave[2]->connect("localhost", 2701);

//   Workrave::Ptr workrave[3] = Workrave::create();
//   workrave[3]->connect("localhost", 2703);
//   workrave[3]->connect("localhost", 2701);
  
//   g_timeout_add_seconds(1, on_timer, workrave[0].get());
//   g_timeout_add_seconds(1, on_timer, workrave[1].get());
//   g_timeout_add_seconds(1, on_timer, workrave[2].get());
//   g_timeout_add_seconds(1, on_timer, workrave[3].get());
  
//   g_main_loop_run(loop);
  
//   return 0;
// }
