// Test.cc --- Applet info Window
//
// Copyright (C) 2013 Rob Caelers & Raymond Penners
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

#include <QtCore>
#include <QtDBus>

#define  WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define  WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define  WORKRAVE_TEST_SERVICE "org.workrave.Test"

#define BOOST_TEST_MODULE workravedbus
#include <boost/test/included/unit_test.hpp>

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/signals2.hpp>

#include "debug.hh"

#include "dbus/IDBus.hh"
#include "DBusTestData.hh"

using namespace std;
using namespace workrave::utils;

struct Fixture
{
  Fixture()
  {
#ifdef TRACING
    Debug::init(boost::unit_test::framework::current_test_case().p_name.get() + "-");
    Debug::name(string("main"));
#endif

    int argc = 1;
    char argv1[] = "test";
    char* argv[] = {argv1, NULL};
    app = new QCoreApplication(argc, argv);

    qDBusRegisterMetaType<DBusTestData::StructWithAllBasicTypes>();
    qDBusRegisterMetaType<DBusTestData::Data>();
    qDBusRegisterMetaType<QList<DBusTestData::Data>>();
    qDBusRegisterMetaType<QMap<QString, DBusTestData::Data>>();
  }
  
  ~Fixture()
  {
    BOOST_TEST_MESSAGE("destructing cores");
    BOOST_TEST_MESSAGE("destructing cores...done");

    delete app;
   }

private:
  QCoreApplication *app;
  
};

BOOST_FIXTURE_TEST_SUITE(s, Fixture)

static void
test_dbus_basic(const char *method)
{
  QDBusConnection connection = QDBusConnection::sessionBus();

  DBusTestData::StructWithAllBasicTypes s;
  s.m_int    = -7;               
  s.m_uint8  = 67;               
  s.m_int16  = 2345;             
  s.m_uint16 = 19834;            
  s.m_int32  = 3937628;          
  s.m_uint32 = 45432;            
  s.m_int64  = 46583739;         
  s.m_uint64 = 3439478327;       
  s.m_string = "Hello";          
  s.m_bool   = true;             
  s.m_double = 3.14;             
  s.m_enum   = DBusTestData::TWO;
  
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, method);
  message << QVariant::fromValue((int)             s.m_int    );
  message << QVariant::fromValue((uint8_t)         s.m_uint8  );
  message << QVariant::fromValue((int16_t)         s.m_int16  );
  message << QVariant::fromValue((uint16_t)        s.m_uint16 );
  message << QVariant::fromValue((int32_t)         s.m_int32  );
  message << QVariant::fromValue((uint32_t)        s.m_uint32 );
  message << QVariant::fromValue((qlonglong)       s.m_int64  );
  message << QVariant::fromValue((qulonglong)      s.m_uint64 );
  message << QVariant::fromValue(QString::fromStdString(s.m_string));
  message << QVariant::fromValue((bool)            s.m_bool   );
  message << QVariant::fromValue((double)          s.m_double );
  message << QVariant::fromValue(QString::fromStdString(DBusTestData::enum_to_str(s.m_enum)));
  
  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  
  int                 r_int    = arguments.at(0).value<int>();   
  uint8_t             r_uint8  = arguments.at(1).value<uint8_t>(); 
  int16_t             r_int16  = arguments.at(2).value<int16_t>(); 
  uint16_t            r_uint16 = arguments.at(3).value<uint16_t>();
  int32_t             r_int32  = arguments.at(4).value<int32_t>(); 
  uint32_t            r_uint32 = arguments.at(5).value<uint32_t>();
  int64_t             r_int64  = arguments.at(6).value<qlonglong>();  
  uint64_t            r_uint64 = arguments.at(7).value<qulonglong>();
  std::string         r_string = arguments.at(8).value<QString>().toStdString(); 
  bool                r_bool   = arguments.at(9).value<bool>();  
  double              r_double = arguments.at(10).value<double>();
  DBusTestData::Enum  r_enum   = DBusTestData::str_to_enum(arguments.at(11).value<QString>().toStdString());

  BOOST_CHECK_EQUAL(r_int    , s.m_int    + 1);
  BOOST_CHECK_EQUAL(r_uint8  , s.m_uint8  + 2); 
  BOOST_CHECK_EQUAL(r_int16  , s.m_int16  + 3);
  BOOST_CHECK_EQUAL(r_uint16 , s.m_uint16 + 4);
  BOOST_CHECK_EQUAL(r_int32  , s.m_int32  + 5);
  BOOST_CHECK_EQUAL(r_uint32 , s.m_uint32 + 6);
  BOOST_CHECK_EQUAL(r_int64  , s.m_int64  + 7);
  BOOST_CHECK_EQUAL(r_uint64 , s.m_uint64 + 8);
  BOOST_CHECK_EQUAL(r_string , s.m_string + " World");
  BOOST_CHECK_EQUAL(r_bool   , !s.m_bool);
  BOOST_CHECK_CLOSE(r_double , s.m_double + 1.1, 0.001);
  BOOST_CHECK_EQUAL(r_enum   , s.m_enum   );
}

BOOST_AUTO_TEST_CASE(test_dbus_basic_1)
{
  test_dbus_basic("Basic1");
}

BOOST_AUTO_TEST_CASE(test_dbus_basic_2)
{
  test_dbus_basic("Basic2");
}

static void test_dbus_struct(const char *method)
{
  DBusTestData::StructWithAllBasicTypes s;
  s.m_int    = -7;
  s.m_uint8  = 67;
  s.m_int16  = 2345;
  s.m_uint16 = 19834;
  s.m_int32  = 3937628;
  s.m_uint32 = 45432;
  s.m_int64  = 46583739;
  s.m_uint64 = 3439478327;
  s.m_string = "Hello";
  s.m_bool   = true;
  s.m_double = 3.14;
  s.m_enum   = DBusTestData::TWO;
  
  QDBusConnection con = QDBusConnection::sessionBus();

  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, method);
  msg.setArguments(QVariantList() << QVariant::fromValue(s));
  QDBusMessage reply = con.call(msg);

  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  
  QDBusArgument r_arg  = arguments.at(0).value<QDBusArgument>();   
  DBusTestData::StructWithAllBasicTypes r_struct;
  r_arg >> r_struct;
  
  BOOST_CHECK_EQUAL(r_struct.m_int    , s.m_int    + 1);
  BOOST_CHECK_EQUAL(r_struct.m_uint8  , s.m_uint8  + 2); 
  BOOST_CHECK_EQUAL(r_struct.m_int16  , s.m_int16  + 3);
  BOOST_CHECK_EQUAL(r_struct.m_uint16 , s.m_uint16 + 4);
  BOOST_CHECK_EQUAL(r_struct.m_int32  , s.m_int32  + 5);
  BOOST_CHECK_EQUAL(r_struct.m_uint32 , s.m_uint32 + 6);
  BOOST_CHECK_EQUAL(r_struct.m_int64  , s.m_int64  + 7);
  BOOST_CHECK_EQUAL(r_struct.m_uint64 , s.m_uint64 + 8);
  BOOST_CHECK_EQUAL(r_struct.m_string , s.m_string + " World");
  BOOST_CHECK_EQUAL(r_struct.m_bool   , !s.m_bool);
  BOOST_CHECK_CLOSE(r_struct.m_double , s.m_double + 1.1, 0.001);
  BOOST_CHECK_EQUAL(r_struct.m_enum   , s.m_enum   );
}

BOOST_AUTO_TEST_CASE(test_dbus_struct_1)
{
  test_dbus_struct("Struct1");
}

BOOST_AUTO_TEST_CASE(test_dbus_struct_2)
{
  test_dbus_struct("Struct2");
}


BOOST_AUTO_TEST_CASE(test_dbus_list_of_struct)
{
  DBusTestData::Data d1(1, 2);
  DBusTestData::Data d2(10, 27);
  DBusTestData::Data d3(12, 26);
  DBusTestData::Data d4(13, 24);

  QList<DBusTestData::Data> datalist;
  datalist.append(d1);
  datalist.append(d2);
  datalist.append(d3);
  datalist.append(d4);

  QDBusConnection con = QDBusConnection::sessionBus();

  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, "List1");
  msg.setArguments(QVariantList() << QVariant::fromValue(datalist));
  QDBusMessage reply = con.call(msg);

  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  
  QDBusArgument r_arg  = arguments.at(0).value<QDBusArgument>();   
  QList<DBusTestData::Data> r_data;
  r_arg >> r_data;

  BOOST_CHECK_EQUAL(r_data.count(), datalist.count());
  for (int i = 0; i < datalist.count(); i++)
    {
      BOOST_CHECK_EQUAL(r_data.at(i).m_key, datalist.at(i).m_key);
      BOOST_CHECK_EQUAL(r_data.at(i).m_data, datalist.at(i).m_data + 76);
    }
}

BOOST_AUTO_TEST_CASE(test_dbus_map_of_struct)
{
  DBusTestData::Data d1(1, 2);
  DBusTestData::Data d2(10, 27);
  DBusTestData::Data d3(12, 26);
  DBusTestData::Data d4(13, 24);

  QMap<QString, DBusTestData::Data> datamap;
  datamap[QString("1")] = d1;
  datamap[QString("2")] = d2;
  datamap[QString("3")] = d3;
  datamap[QString("4")] = d4;

  QDBusConnection con = QDBusConnection::sessionBus();

  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, "Map1");
  msg.setArguments(QVariantList() << QVariant::fromValue(datamap));
  QDBusMessage reply = con.call(msg);

  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  
  QDBusArgument r_arg  = arguments.at(0).value<QDBusArgument>();   
  QMap<QString, DBusTestData::Data> r_data;
  r_arg >> r_data;

  BOOST_CHECK_EQUAL(r_data.count(), datamap.count());
  for (QString &k : datamap.keys())
    {
      BOOST_CHECK_EQUAL(r_data[k].m_data, datamap[k].m_data + 65);
    }
}

BOOST_AUTO_TEST_SUITE_END()
