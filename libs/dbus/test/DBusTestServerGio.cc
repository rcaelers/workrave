// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#include "debug.hh"

#include "DBusTestServerGio.hh"
#include "DBusTestGio.hh"

#include "DBusGio.hh"

#define WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define WORKRAVE_TEST_SERVICE "org.workrave.Test"

using namespace std;

//! Constructor.
DBusTestServerGio::DBusTestServerGio()
{
}

//! Destructor.
DBusTestServerGio::~DBusTestServerGio()
{
}

int
main(int argc, char **argv)
{
  DBusTestServerGio s;
  s.run(argc, argv);
}

void
DBusTestServerGio::run(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  try
    {
      GMainLoop *loop = g_main_loop_new(NULL, TRUE);

      dbus = workrave::dbus::DBusGio::create();

      dbus->init();

      extern void init_DBusTestGio(workrave::dbus::IDBus::Ptr dbus);
      init_DBusTestGio(dbus);

      dbus->connect(WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, this);
      dbus->register_object_path(WORKRAVE_TEST_PATH);
      dbus->register_service(WORKRAVE_TEST_SERVICE);

      g_main_loop_run(loop);
      g_main_loop_unref(loop);
    }
  catch (workrave::dbus::DBusException &)
    {
      std::cerr << "server failed" << std::endl;
    }
}

void
DBusTestServerGio::test_fire_signal()
{
  org_workrave_TestInterface *test = org_workrave_TestInterface::instance(dbus);

  DBusTestData::StructWithAllBasicTypes par;
  par.m_int = -7;
  par.m_uint8 = 67;
  par.m_int16 = 2345;
  par.m_uint16 = 19834;
  par.m_int32 = 3937628;
  par.m_uint32 = 45432;
  par.m_int64 = 46583739;
  par.m_uint64 = 3439478327;
  par.m_string = "Hello";
  par.m_bool = true;
  par.m_double = 3.14;
  par.m_enum = DBusTestData::TWO;

  DBusTestData::DataList l;
  DBusTestData::DataMap m;

  if (test != NULL)
    {
      test->Signal("/org/workrave/Test",
                   par.m_int,
                   par.m_uint8,
                   par.m_int16,
                   par.m_uint16,
                   par.m_int32,
                   par.m_uint32,
                   par.m_int64,
                   par.m_uint64,
                   par.m_string,
                   par.m_bool,
                   par.m_double,
                   par.m_enum,
                   l,
                   m);
    }
}

void
DBusTestServerGio::test_fire_signal_without_args()
{
  org_workrave_TestInterface *test = org_workrave_TestInterface::instance(dbus);

  if (test != NULL)
    {
      test->SignalWithoutArgs("/org/workrave/Test");
    }
}

void
DBusTestServerGio::test_fire_signal_with_ref()
{
  org_workrave_TestInterface *test = org_workrave_TestInterface::instance(dbus);

  DBusTestData::StructWithAllBasicTypes par;
  par.m_int = -7;
  par.m_uint8 = 67;
  par.m_int16 = 2345;
  par.m_uint16 = 19834;
  par.m_int32 = 3937628;
  par.m_uint32 = 45432;
  par.m_int64 = 46583739;
  par.m_uint64 = 3439478327;
  par.m_string = "Hello";
  par.m_bool = true;
  par.m_double = 3.14;
  par.m_enum = DBusTestData::TWO;

  DBusTestData::DataList l;
  DBusTestData::DataMap m;

  if (test != NULL)
    {
      test->SignalWithRef("/org/workrave/Test",
                          par.m_int,
                          par.m_uint8,
                          par.m_int16,
                          par.m_uint16,
                          par.m_int32,
                          par.m_uint32,
                          par.m_int64,
                          par.m_uint64,
                          par.m_string,
                          par.m_bool,
                          par.m_double,
                          par.m_enum,
                          l,
                          m);
    }
}
