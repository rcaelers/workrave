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

#include <QtCore>
#include <QtDBus>

#include "debug.hh"

#include "DBusTestServerQt.hh"
#include "DBusTestQt.hh"
#include "DBusTestDataMeta.hh"

#include "dbus/IDBus.hh"

#include "DBusQt.hh"

#define WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define WORKRAVE_TEST_SERVICE "org.workrave.Test"

using namespace std;
using namespace org::workrave::test;

int
main(int argc, char **argv)
{
  DBusTestServerQt s;
  s.run(argc, argv);
}

void
DBusTestServerQt::run(int argc, char **argv)
{
  try
    {
      app = new QCoreApplication(argc, argv);

      qDBusRegisterMetaType<DBusTestData::StructWithAllBasicTypes>();
      qDBusRegisterMetaType<DBusTestData::StructWithAllBasicTypesReorder>();
      qDBusRegisterMetaType<DBusTestData::Data>();
      qDBusRegisterMetaType<QList<DBusTestData::Data>>();
      qDBusRegisterMetaType<std::list<DBusTestData::Data>>();
      qDBusRegisterMetaType<QMap<QString, DBusTestData::Data>>();

      dbus = std::make_shared<workrave::dbus::DBusQt>();

      dbus->init();

      extern void init_DBusTestQt(std::shared_ptr<workrave::dbus::IDBus> dbus);
      init_DBusTestQt(dbus);

      dbus->connect(WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, this);
      dbus->register_object_path(WORKRAVE_TEST_PATH);
      dbus->register_service(WORKRAVE_TEST_SERVICE);

      app->exec();
    }
  catch (workrave::dbus::DBusException &)
    {
      std::cerr << "server failed" << std::endl;
    }
}
void
DBusTestServerQt::test_fire_signal()
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

  l.emplace_back(1, 2);
  m["1"] = DBusTestData::Data(1, 2);

  if (test != nullptr)
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
DBusTestServerQt::test_fire_signal_without_args()
{
  org_workrave_TestInterface *test = org_workrave_TestInterface::instance(dbus);

  if (test != nullptr)
    {
      test->SignalWithoutArgs("/org/workrave/Test");
    }
}

void
DBusTestServerQt::test_fire_signal_with_ref()
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

  l.emplace_back(1, 2);
  m["1"] = DBusTestData::Data(1, 2);

  if (test != nullptr)
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
