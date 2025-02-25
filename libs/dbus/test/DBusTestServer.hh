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

#ifndef DBUSTESTSERVER_HH
#define DBUSTESTSERVER_HH

#include <string>
#include <set>

#if defined(DBUS_BACKEND_QT)
#  include <QDBusArgument>
#  include <QMetaType>
#elif defined(DBUS_BACKEND_GIO)
#  include <glib.h>
#endif

#include "DBusTestData.hh"

class DBusTestServer
{
public:
  DBusTestServer() = default;
  virtual ~DBusTestServer() = default;

  virtual void run(int argc, char **argv) = 0;

  // DBus
  void test_basic_out_ref(int i_int,
                          uint8_t i_uint8,
                          int16_t i_int16,
                          uint16_t i_uint16,
                          int32_t i_int32,
                          uint32_t i_uint32,
                          int64_t i_int64,
                          uint64_t i_uint64,
                          std::string i_string,
                          bool i_bool,
                          double i_double,
                          DBusTestData::Enum i_enum,
                          int &o_int,
                          uint8_t &o_uint8,
                          int16_t &o_int16,
                          uint16_t &o_uint16,
                          int32_t &o_int32,
                          uint32_t &o_uint32,
                          int64_t &o_int64,
                          uint64_t &o_uint64,
                          std::string &o_string,
                          bool &o_bool,
                          double &o_double,
                          DBusTestData::Enum &o_enum);

  void test_basic_out_ptr(int i_int,
                          uint8_t i_uint8,
                          int16_t i_int16,
                          uint16_t i_uint16,
                          int32_t i_int32,
                          uint32_t i_uint32,
                          int64_t i_int64,
                          uint64_t i_uint64,
                          const std::string &i_string,
                          bool i_bool,
                          double i_double,
                          DBusTestData::Enum i_enum,
                          int *o_int,
                          uint8_t *o_uint8,
                          int16_t *o_int16,
                          uint16_t *o_uint16,
                          int32_t *o_int32,
                          uint32_t *o_uint32,
                          int64_t *o_int64,
                          uint64_t *o_uint64,
                          std::string *o_string,
                          bool *o_bool,
                          double *o_double,
                          DBusTestData::Enum *o_enum);

  std::string test_return_string(int i_int);
  int test_return_int(int i_int);
  DBusTestData::DataList test_return_list();

  void test_struct_out_ref(const DBusTestData::StructWithAllBasicTypes &i_struct,
                           DBusTestData::StructWithAllBasicTypes &o_struct);

  void test_struct_out_ptr(DBusTestData::StructWithAllBasicTypes i_struct, DBusTestData::StructWithAllBasicTypes *o_struct);

  void test_list_of_struct(DBusTestData::DataList i_data, DBusTestData::DataList &o_data);

  void test_map_of_struct(DBusTestData::DataMap i_data, DBusTestData::DataMap &o_data);

  virtual void test_fire_signal() = 0;
  virtual void test_fire_signal_without_args() = 0;
  virtual void test_fire_signal_with_ref() = 0;
};

#endif // DBUSTESTSERVER_HH
