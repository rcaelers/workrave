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

    //system("netstat -nat");
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

        cloud[i] = workraves[i]->get_cloud();
        cloud_test[i] = boost::dynamic_pointer_cast<ICloudTest>(cloud[i]);
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
  /*   /---\
   *   | 0 |
   *   \---/
   *     |
   *     |------|
   *     |      |
   *   /---\  /---\
   *   | 1 |  | 2 |
   *   \---/  \---/
   *     |      |
   *     |      |
   *   /---\  /---\
   *   | 3 |  | 4 |
   *   \---/  \---/
   *     |
   *     |
   *   /---\
   *   | 5 |
   *   \---/
   *     |
   *     |------|
   *     |      |
   *   /---\  /---\
   *   | 6 |  | 7 |
   *   \---/  \---/
   */

  void connect_all()
  {
    BOOST_TEST_MESSAGE("connecting cores...");
    for (int i = 0; i <= 6; i++)
      {
        connect_one(i);
      }
    sleep(5);
    BOOST_TEST_MESSAGE("connecting cores...done");
  }

  void connect_all_delayed()
  {
    BOOST_TEST_MESSAGE("connecting cores...");
    for (int i = 0; i <= 6; i++)
      {
        connect_one(i);
        g_usleep(G_USEC_PER_SEC);
      }
    sleep(5);
    BOOST_TEST_MESSAGE("connecting cores...done");
  }
  
  void connect_all_shuffled()
  {
    BOOST_TEST_MESSAGE("connecting cores...");
    int order[] = { 5,2,4,3,6,0,1 };
    
    for (int i = 0; i <= 6; i++)
      {
        connect_one(order[i]);
        g_usleep(G_USEC_PER_SEC);
      }
    sleep(5);
    BOOST_TEST_MESSAGE("connecting cores...done");
  }
  
  void connect_one(int c)
  {
    BOOST_TEST_MESSAGE("connect " + boost::lexical_cast<string>(c));
    switch(c)
      {
      case 0:
        workraves[1]->invoke(boost::bind(&Workrave::connect, workraves[1], "localhost", 2700));
        break;

      case 1:
        workraves[2]->invoke(boost::bind(&Workrave::connect, workraves[2], "localhost", 2700));
        break;

      case 2:
        workraves[3]->invoke(boost::bind(&Workrave::connect, workraves[3], "localhost", 2701));
        break;

      case 3:
        workraves[4]->invoke(boost::bind(&Workrave::connect, workraves[4], "localhost", 2702));
        break;

      case 4:
        workraves[5]->invoke(boost::bind(&Workrave::connect, workraves[5], "localhost", 2703));
        break;

      case 5:
        workraves[6]->invoke(boost::bind(&Workrave::connect, workraves[6], "localhost", 2705));
        break;
        
      case 6:
        workraves[7]->invoke(boost::bind(&Workrave::connect, workraves[7], "localhost", 2705));
        break;
      }
   
  } 
  
  const static int num_workraves = 8;
  Workrave::Ptr workraves[num_workraves];
  ICore::Ptr cores[num_workraves];
  ICloud::Ptr cloud[num_workraves];
  ICloudTest::Ptr cloud_test[num_workraves];
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


BOOST_AUTO_TEST_CASE(test_connect_delayed)
{
  connect_all_delayed();

  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);
    }
}

BOOST_AUTO_TEST_CASE(test_connect_simultaneuously)
{
  connect_all();

  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);
    }
}

BOOST_AUTO_TEST_CASE(test_connect_shuffled)
{
  connect_all_shuffled();

  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);
    }
}

BOOST_AUTO_TEST_CASE(test_disconnect)
{
  connect_all();

  UUID id = workraves[1]->invoke_sync(boost::bind(&ICloudTest::get_id, cloud_test[1]));

  workraves[3]->invoke(boost::bind(&ICloudTest::disconnect, cloud_test[3], id));
  sleep(5);

  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 3);
    }
}


BOOST_AUTO_TEST_CASE(test_auto_connect)
{
  for (int i = 0; i < num_workraves; i++)
    {
      workraves[i]->invoke_sync(boost::bind(&ICloudTest::start_announce, cloud_test[i]));
    }

  sleep(10);

  int total_direct = 0;
  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);

      int cycles = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_cycle_failures, cloud_test[i]));
      BOOST_CHECK_EQUAL(cycles, 0);

      total_direct += workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_direct_clients, cloud_test[i])).size();
    }
  BOOST_CHECK_EQUAL(total_direct, 14);
}

BOOST_AUTO_TEST_CASE(test_auto_connect_partial1)
{
  connect_one(0);
  connect_one(1);
  connect_one(2);
  sleep(5);
  
  for (int i = 0; i < num_workraves; i++)
    {
      workraves[i]->invoke_sync(boost::bind(&ICloudTest::start_announce, cloud_test[i]));
    }

  sleep(10);

  int total_direct = 0;
  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);

      int cycles = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_cycle_failures, cloud_test[i]));
      BOOST_CHECK_EQUAL(cycles, 0);

       total_direct += workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_direct_clients, cloud_test[i])).size();
    }
  BOOST_CHECK_EQUAL(total_direct, 14);
}

BOOST_AUTO_TEST_CASE(test_auto_connect_partial2)
{
  connect_one(5);
  connect_one(6);
  connect_one(7);
  sleep(5);
  
  for (int i = 0; i < num_workraves; i++)
    {
      workraves[i]->invoke_sync(boost::bind(&ICloudTest::start_announce, cloud_test[i]));
    }

  sleep(10);

  int total_direct = 0;
  for (int i = 0; i < num_workraves; i++)
    {
      list<UUID> ids = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_clients, cloud_test[i]));      
      BOOST_CHECK_EQUAL(ids.size(), 7);

      int cycles = workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_cycle_failures, cloud_test[i]));
      BOOST_CHECK_EQUAL(cycles, 0);

      total_direct += workraves[i]->invoke_sync(boost::bind(&ICloudTest::get_direct_clients, cloud_test[i])).size();
    }
  BOOST_CHECK_EQUAL(total_direct, 14);
}

BOOST_AUTO_TEST_SUITE_END()
