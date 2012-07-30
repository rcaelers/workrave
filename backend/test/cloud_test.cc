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
#ifdef TRACING
    Debug::init(boost::unit_test::framework::current_test_case().p_name.get() + "-");
    Debug::name(string("main"));
#endif

    XInitThreads();
    g_type_init();
    gtk_init(0, 0);

    system("netstat -nat");
    BOOST_TEST_MESSAGE("constructing cores");
    
    barrier = boost::shared_ptr<boost::barrier>(new boost::barrier(num_workraves + 1));

    for (int i = 0; i < num_workraves; i++)
      {
        workraves[i] = Workrave::create(i);
        workraves[i]->init(barrier);
      }

    barrier->wait();
    BOOST_TEST_MESSAGE("constructing cores...done");
    
    for (int i = 0; i < num_workraves; i++)
      {
        cores[i] = workraves[i]->get_core();
      }
  }
  
  ~Fixture()
  {
    BOOST_TEST_MESSAGE("destructing cores");
  
    for (int i = 0; i < num_workraves; i++)
      {
        //workraves[i]->invoke_sync(boost::bind(&Workrave::terminate, workraves[i]));
        workraves[i]->terminate();
      }
    sleep(5);
    
    //barrier->wait();
    BOOST_TEST_MESSAGE("destructing cores...done");
   }

protected:
  void connect_all()
  {
    /*   /---\
       | 0 |
       \---/
         |
         |------|
         |      |
       /---\  /---\
       | 1 |  | 2 |
       \---/  \---/
         |      |
         |      |
       /---\  /---\
       | 3 |  | 4 |
       \---/  \---/
         |
         |
       /---\
       | 5 |
       \---/
         |
         |------|
         |      |
       /---\  /---\
       | 6 |  | 7 |
       \---/  \---/
    */

    BOOST_TEST_MESSAGE("connecting cores...");
    
    workraves[1]->invoke(boost::bind(&Workrave::connect, workraves[1], "localhost", 2700));
    workraves[2]->invoke(boost::bind(&Workrave::connect, workraves[2], "localhost", 2700));
    
    workraves[3]->invoke(boost::bind(&Workrave::connect, workraves[3], "localhost", 2701));
    workraves[4]->invoke(boost::bind(&Workrave::connect, workraves[4], "localhost", 2702));

    workraves[5]->invoke(boost::bind(&Workrave::connect, workraves[5], "localhost", 2703));
    
    workraves[6]->invoke(boost::bind(&Workrave::connect, workraves[6], "localhost", 2705));
    workraves[7]->invoke(boost::bind(&Workrave::connect, workraves[7], "localhost", 2705));
    sleep(5);
    BOOST_TEST_MESSAGE("connecting cores...done");
  } 
  
  const static int num_workraves = 8;
  Workrave::Ptr workraves[num_workraves];
  ICore::Ptr cores[num_workraves];
  boost::shared_ptr<boost::barrier> barrier;
};

BOOST_FIXTURE_TEST_SUITE(s, Fixture)

BOOST_AUTO_TEST_CASE(test_propagate_operation_mode)
{
  connect_all();
  
  BOOST_TEST_MESSAGE("checking initial...");
  for (int i = 0; i <  num_workraves; i++)
    {
      OperationMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_operation_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);
    }
  
  BOOST_TEST_MESSAGE("set operation mode to suspended");
  workraves[0]->invoke_sync(boost::bind(&ICore::set_operation_mode, cores[0], OPERATION_MODE_SUSPENDED));
  sleep(2);

  for (int i = 0; i <  num_workraves; i++)
    {
      OperationMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_operation_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, OPERATION_MODE_SUSPENDED);
    }

  BOOST_TEST_MESSAGE("set operation mode to quiet");
  workraves[0]->invoke_sync(boost::bind(&ICore::set_operation_mode, cores[0], OPERATION_MODE_QUIET));
  sleep(2);

  for (int i = 0; i <  num_workraves; i++)
    {
      OperationMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_operation_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, OPERATION_MODE_QUIET);
    }

  BOOST_TEST_MESSAGE("set operation mode to normal");
  workraves[0]->invoke_sync(boost::bind(&ICore::set_operation_mode, cores[0], OPERATION_MODE_NORMAL));
  sleep(2);

  for (int i = 0; i <  num_workraves; i++)
    {
      OperationMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_operation_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, OPERATION_MODE_NORMAL);
    }
  BOOST_TEST_MESSAGE("done");
}


BOOST_AUTO_TEST_CASE(test_propagate_usage_mode)
{
  connect_all();
  
  for (int i = 0; i <  num_workraves; i++)
    {
      UsageMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_usage_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, USAGE_MODE_NORMAL);
    }
  
  workraves[0]->invoke_sync(boost::bind(&ICore::set_usage_mode, cores[0], USAGE_MODE_READING));
  sleep(2);

  for (int i = 0; i <  num_workraves; i++)
    {
      UsageMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_usage_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, USAGE_MODE_READING);
    }

  workraves[0]->invoke_sync(boost::bind(&ICore::set_usage_mode, cores[0], USAGE_MODE_NORMAL));
  sleep(2);

  for (int i = 0; i <  num_workraves; i++)
    {
      UsageMode mode = workraves[i]->invoke_sync(boost::bind(&ICore::get_usage_mode, cores[i]));
      BOOST_CHECK_EQUAL(mode, USAGE_MODE_NORMAL);
    }
  
}

BOOST_AUTO_TEST_SUITE_END()
