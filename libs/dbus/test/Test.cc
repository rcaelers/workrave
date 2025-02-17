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

#define WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define WORKRAVE_TEST_SERVICE "org.workrave.Test"

#define BOOST_TEST_MODULE workravedbus
#include <boost/test/unit_test.hpp>

#include <thread>
#include <boost/signals2.hpp>

#include "debug.hh"

#include "dbus/IDBus.hh"
#include "DBusTestData.hh"
#include "DBusTestDataMeta.hh"
#include "Test.hh"

using namespace std;
using namespace workrave::utils;


struct Fixture
{
  Fixture()
  {
    int argc = 1;
    char argv1[] = "test";
    char *argv[] = {argv1, nullptr};
    app = new QCoreApplication(argc, argv);

    qDBusRegisterMetaType<DBusTestData::StructWithAllBasicTypes>();
    qDBusRegisterMetaType<DBusTestData::StructWithAllBasicTypesReorder>();
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
  DBusTestData::StructWithAllBasicTypes inpar;
  inpar.m_int = -7;
  inpar.m_uint8 = 67;
  inpar.m_int16 = 2345;
  inpar.m_uint16 = 19834;
  inpar.m_int32 = 3937628;
  inpar.m_uint32 = 45432;
  inpar.m_int64 = 46583739;
  inpar.m_uint64 = 3439478327;
  inpar.m_string = "Hello";
  inpar.m_bool = true;
  inpar.m_double = 3.14;
  inpar.m_enum = DBusTestData::TWO;

  QDBusConnection connection = QDBusConnection::sessionBus();
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        method);
  message << QVariant::fromValue(static_cast<int>(inpar.m_int));
  message << QVariant::fromValue(static_cast<uint8_t>(inpar.m_uint8));
  message << QVariant::fromValue(static_cast<int16_t>(inpar.m_int16));
  message << QVariant::fromValue(static_cast<uint16_t>(inpar.m_uint16));
  message << QVariant::fromValue(static_cast<int32_t>(inpar.m_int32));
  message << QVariant::fromValue(static_cast<uint32_t>(inpar.m_uint32));
  message << QVariant::fromValue(static_cast<qlonglong>(inpar.m_int64));
  message << QVariant::fromValue(static_cast<qulonglong>(inpar.m_uint64));
  message << QVariant::fromValue(QString::fromStdString(inpar.m_string));
  message << QVariant::fromValue(static_cast<bool>(inpar.m_bool));
  message << QVariant::fromValue(static_cast<double>(inpar.m_double));
  message << QVariant::fromValue(QString::fromStdString(DBusTestData::enum_to_str(inpar.m_enum)));

  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 12);

  int r_int = arguments.at(0).value<int>();
  uint8_t r_uint8 = arguments.at(1).value<uint8_t>();
  int16_t r_int16 = arguments.at(2).value<int16_t>();
  uint16_t r_uint16 = arguments.at(3).value<uint16_t>();
  int32_t r_int32 = arguments.at(4).value<int32_t>();
  uint32_t r_uint32 = arguments.at(5).value<uint32_t>();
  int64_t r_int64 = arguments.at(6).value<qlonglong>();
  uint64_t r_uint64 = arguments.at(7).value<qulonglong>();
  std::string r_string = arguments.at(8).value<QString>().toStdString();
  bool r_bool = arguments.at(9).value<bool>();
  double r_double = arguments.at(10).value<double>();
  DBusTestData::Enum r_enum = DBusTestData::str_to_enum(arguments.at(11).value<QString>().toStdString());

  BOOST_CHECK_EQUAL(r_int, inpar.m_int + 1);
  BOOST_CHECK_EQUAL(r_uint8, inpar.m_uint8 + 2);
  BOOST_CHECK_EQUAL(r_int16, inpar.m_int16 + 3);
  BOOST_CHECK_EQUAL(r_uint16, inpar.m_uint16 + 4);
  BOOST_CHECK_EQUAL(r_int32, inpar.m_int32 + 5);
  BOOST_CHECK_EQUAL(r_uint32, inpar.m_uint32 + 6);
  BOOST_CHECK_EQUAL(r_int64, inpar.m_int64 + 7);
  BOOST_CHECK_EQUAL(r_uint64, inpar.m_uint64 + 8);
  BOOST_CHECK_EQUAL(r_string, inpar.m_string + " World");
  BOOST_CHECK_EQUAL(r_bool, !inpar.m_bool);
  BOOST_CHECK_CLOSE(r_double, inpar.m_double + 1.1, 0.001);
  BOOST_CHECK_EQUAL(r_enum, inpar.m_enum);
}

BOOST_AUTO_TEST_CASE(test_dbus_basic_out_ref)
{
  test_dbus_basic("BasicOutRef");
}

BOOST_AUTO_TEST_CASE(test_dbus_basic_out_ptr)
{
  test_dbus_basic("BasicOutPtr");
}

static void
test_dbus_struct(const char *method)
{
  DBusTestData::StructWithAllBasicTypes inpar;
  inpar.m_int = -7;
  inpar.m_uint8 = 67;
  inpar.m_int16 = 2345;
  inpar.m_uint16 = 19834;
  inpar.m_int32 = 3937628;
  inpar.m_uint32 = 45432;
  inpar.m_int64 = 46583739;
  inpar.m_uint64 = 3439478327;
  inpar.m_string = "Hello";
  inpar.m_bool = true;
  inpar.m_double = 3.14;
  inpar.m_enum = DBusTestData::TWO;

  QDBusConnection con = QDBusConnection::sessionBus();
  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, method);
  msg.setArguments(QVariantList() << QVariant::fromValue(inpar));

  QDBusMessage reply = con.call(msg);
  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  QDBusArgument r_arg = arguments.at(0).value<QDBusArgument>();
  DBusTestData::StructWithAllBasicTypes r_struct;
  r_arg >> r_struct;

  BOOST_CHECK_EQUAL(r_struct.m_int, inpar.m_int + 1);
  BOOST_CHECK_EQUAL(r_struct.m_uint8, inpar.m_uint8 + 2);
  BOOST_CHECK_EQUAL(r_struct.m_int16, inpar.m_int16 + 3);
  BOOST_CHECK_EQUAL(r_struct.m_uint16, inpar.m_uint16 + 4);
  BOOST_CHECK_EQUAL(r_struct.m_int32, inpar.m_int32 + 5);
  BOOST_CHECK_EQUAL(r_struct.m_uint32, inpar.m_uint32 + 6);
  BOOST_CHECK_EQUAL(r_struct.m_int64, inpar.m_int64 + 7);
  BOOST_CHECK_EQUAL(r_struct.m_uint64, inpar.m_uint64 + 8);
  BOOST_CHECK_EQUAL(r_struct.m_string, inpar.m_string + " World");
  BOOST_CHECK_EQUAL(r_struct.m_bool, !inpar.m_bool);
  BOOST_CHECK_CLOSE(r_struct.m_double, inpar.m_double + 1.1, 0.001);
  BOOST_CHECK_EQUAL(r_struct.m_enum, inpar.m_enum);
}

BOOST_AUTO_TEST_CASE(test_dbus_struct_out_refo)
{
  test_dbus_struct("StructOutPtr");
}

BOOST_AUTO_TEST_CASE(test_dbus_struct_out_ptr)
{
  test_dbus_struct("StructOutPtr");
}

BOOST_AUTO_TEST_CASE(test_dbus_list_of_struct)
{
  QList<DBusTestData::Data> inpar;
  inpar.append(DBusTestData::Data(1, 2));
  inpar.append(DBusTestData::Data(10, 27));
  inpar.append(DBusTestData::Data(12, 26));
  inpar.append(DBusTestData::Data(13, 24));

  QDBusConnection con = QDBusConnection::sessionBus();
  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, "List");
  msg.setArguments(QVariantList() << QVariant::fromValue(inpar));

  QDBusMessage reply = con.call(msg);
  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  QDBusArgument r_arg = arguments.at(0).value<QDBusArgument>();
  QList<DBusTestData::Data> r_data;
  r_arg >> r_data;

  BOOST_CHECK_EQUAL(r_data.count(), inpar.count());
  for (int i = 0; i < inpar.count(); i++)
    {
      BOOST_CHECK_EQUAL(r_data.at(i).m_key, inpar.at(i).m_key);
      BOOST_CHECK_EQUAL(r_data.at(i).m_data, inpar.at(i).m_data + 76);
    }
}

BOOST_AUTO_TEST_CASE(test_dbus_map_of_struct)
{
  QMap<QString, DBusTestData::Data> inpar;
  inpar[QString("1")] = DBusTestData::Data(1, 2);
  inpar[QString("2")] = DBusTestData::Data(10, 27);
  inpar[QString("3")] = DBusTestData::Data(12, 26);
  inpar[QString("4")] = DBusTestData::Data(13, 24);

  QDBusConnection con = QDBusConnection::sessionBus();
  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, "Map");
  msg.setArguments(QVariantList() << QVariant::fromValue(inpar));

  QDBusMessage reply = con.call(msg);
  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  QDBusArgument r_arg = arguments.at(0).value<QDBusArgument>();
  QMap<QString, DBusTestData::Data> r_data;
  r_arg >> r_data;

  BOOST_REQUIRE_EQUAL(r_data.count(), inpar.count());
  for (QString &k: inpar.keys())
    {
      BOOST_CHECK_EQUAL(r_data[k].m_data, inpar[k].m_data + 65);
    }
}

BOOST_AUTO_TEST_CASE(test_return_string)
{
  QDBusConnection connection = QDBusConnection::sessionBus();
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "ReturnString");
  message << QVariant::fromValue(static_cast<int>(42));

  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  std::string r_string = arguments.at(0).value<QString>().toStdString();
  BOOST_CHECK_EQUAL(r_string, "Hello World");
}

BOOST_AUTO_TEST_CASE(test_return_int)
{
  QDBusConnection connection = QDBusConnection::sessionBus();
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "ReturnInt");
  message << QVariant::fromValue(static_cast<int>(42));

  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  int r_int = arguments.at(0).value<int>();
  BOOST_CHECK_EQUAL(r_int, 84);
}

BOOST_AUTO_TEST_CASE(test_return_list)
{
  QDBusConnection connection = QDBusConnection::sessionBus();
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "ReturnList");

  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);

  QList<QVariant> arguments = reply.arguments();
  BOOST_REQUIRE_EQUAL(arguments.count(), 1);

  QDBusArgument r_arg = arguments.at(0).value<QDBusArgument>();
  QList<DBusTestData::Data> r_data;
  r_arg >> r_data;

  QList<DBusTestData::Data> expected;
  expected.append(DBusTestData::Data(0, 1));
  expected.append(DBusTestData::Data(1, 2));
  expected.append(DBusTestData::Data(3, 5));
  expected.append(DBusTestData::Data(8, 13));

  BOOST_CHECK_EQUAL(r_data.count(), expected.count());
  for (int i = 0; i < expected.count(); i++)
    {
      BOOST_CHECK_EQUAL(r_data.at(i).m_key, expected.at(i).m_key);
      BOOST_CHECK_EQUAL(r_data.at(i).m_data, expected.at(i).m_data);
    }
}

BOOST_AUTO_TEST_CASE(test_test_signal_without_args)
{
  SignalReceiver r;
  QDBusConnection connection = QDBusConnection::sessionBus();

  connection.connect(WORKRAVE_TEST_SERVICE,
                     WORKRAVE_TEST_PATH,
                     WORKRAVE_TEST_INTERFACE,
                     "SignalWithoutArgs",
                     &r,
                     SLOT(on_signal_without_args()));

  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "FireSignalWithoutArgs");
  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);
}

BOOST_AUTO_TEST_CASE(test_test_signal)
{
  SignalReceiver r;
  QDBusConnection connection = QDBusConnection::sessionBus();

  connection.connect(WORKRAVE_TEST_SERVICE, WORKRAVE_TEST_PATH, WORKRAVE_TEST_INTERFACE, "Signal", &r, SLOT(on_signal()));

  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "FireSignal");
  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ReplyMessage);
}

BOOST_AUTO_TEST_CASE(test_test_error_basic)
{
  DBusTestData::StructWithAllBasicTypes inpar;
  inpar.m_int = -7;
  inpar.m_uint8 = 67;
  inpar.m_int16 = 2345;
  inpar.m_uint16 = 19834;
  inpar.m_int32 = 3937628;
  inpar.m_uint32 = 45432;
  inpar.m_int64 = 46583739;
  inpar.m_uint64 = 3439478327;
  inpar.m_string = "Hello";
  inpar.m_bool = true;
  inpar.m_double = 3.14;
  inpar.m_enum = DBusTestData::TWO;

  QDBusConnection connection = QDBusConnection::sessionBus();
  QDBusMessage message = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                        WORKRAVE_TEST_PATH,
                                                        WORKRAVE_TEST_INTERFACE,
                                                        "BasicOutRef");
  message << QVariant::fromValue(static_cast<int>(inpar.m_int));
  message << QVariant::fromValue(static_cast<uint8_t>(inpar.m_uint8));
  message << QVariant::fromValue(static_cast<int16_t>(inpar.m_int16));
  message << QVariant::fromValue(QString::fromStdString(inpar.m_string));
  message << QVariant::fromValue(static_cast<uint16_t>(inpar.m_uint16));
  message << QVariant::fromValue(static_cast<int32_t>(inpar.m_int32));
  message << QVariant::fromValue(static_cast<uint32_t>(inpar.m_uint32));
  message << QVariant::fromValue(static_cast<qlonglong>(inpar.m_int64));
  message << QVariant::fromValue(static_cast<qulonglong>(inpar.m_uint64));
  message << QVariant::fromValue(static_cast<bool>(inpar.m_bool));
  message << QVariant::fromValue(static_cast<double>(inpar.m_double));
  message << QVariant::fromValue(QString::fromStdString(DBusTestData::enum_to_str(inpar.m_enum)));

  QDBusMessage reply = connection.call(message);
  BOOST_REQUIRE_EQUAL(reply.type(), QDBusMessage::ErrorMessage);
}

BOOST_AUTO_TEST_CASE(test_test_error_struct)
{
  DBusTestData::StructWithAllBasicTypesReorder inpar;
  inpar.m_int = -7;
  inpar.m_uint8 = 67;
  inpar.m_int16 = 2345;
  inpar.m_uint16 = 19834;
  inpar.m_int32 = 3937628;
  inpar.m_uint32 = 45432;
  inpar.m_int64 = 46583739;
  inpar.m_uint64 = 3439478327;
  inpar.m_string = "Hello";
  inpar.m_bool = true;
  inpar.m_double = 3.14;
  inpar.m_enum = DBusTestData::TWO;

  QDBusConnection con = QDBusConnection::sessionBus();
  QDBusMessage msg = QDBusMessage::createMethodCall(WORKRAVE_TEST_SERVICE,
                                                    WORKRAVE_TEST_PATH,
                                                    WORKRAVE_TEST_INTERFACE,
                                                    "StructOutRef");
  msg.setArguments(QVariantList() << QVariant::fromValue(inpar));

  QDBusMessage reply = con.call(msg);
  BOOST_CHECK_EQUAL(reply.type(), QDBusMessage::ErrorMessage);
}

BOOST_AUTO_TEST_SUITE_END()
