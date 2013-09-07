// DBusTestServer.cc
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

#include "debug.hh"

#include "DBusTestServer.hh"

#include "dbus/IDBus.hh"

#if defined(DBUS_BACKEND_QT5)
#include "DBusQt5.hh"
#elif defined(DBUS_BACKEND_GIO)
#undef signals
#include "DBusGio.hh"
#endif

#define  WORKRAVE_TEST_PATH "/org/workrave/Workrave/Test"
#define  WORKRAVE_TEST_INTERFACE "org.workrave.TestInterface"
#define  WORKRAVE_TEST_SERVICE "org.workrave.Test"

using namespace std;

//! Constructor.
DBusTestServer::DBusTestServer() 
{
}


//! Destructor.
DBusTestServer::~DBusTestServer()
{
}

// DBus
void DBusTestServer::test_basic_1(int       i_int,
                                  uint8_t   i_uint8,
                                  int16_t   i_int16,
                                  uint16_t  i_uint16,
                                  int32_t   i_int32,
                                  uint32_t  i_uint32,
                                  int64_t   i_int64,
                                  uint64_t  i_uint64,
                                  string    i_string,
                                  bool      i_bool,
                                  double    i_double,
                                  DBusTestData::Enum      i_enum,
                                  int       &o_int,   
                                  uint8_t   &o_uint8, 
                                  int16_t   &o_int16, 
                                  uint16_t  &o_uint16,
                                  int32_t   &o_int32, 
                                  uint32_t  &o_uint32,
                                  int64_t   &o_int64, 
                                  uint64_t  &o_uint64,
                                  string    &o_string,
                                  bool      &o_bool,  
                                  double    &o_double,
                                  DBusTestData::Enum      &o_enum
                                  )
{
  o_int = i_int + 1;
  o_uint8 = i_uint8 + 2;
  o_int16 = i_int16 + 3;
  o_uint16 = i_uint16 + 4;
  o_int32 = i_int32 + 5;
  o_uint32 = i_uint32 + 6;
  o_int64 = i_int64 + 7;
  o_uint64 = i_uint64 + 8;
  o_string = i_string + " World";
  o_bool = !i_bool;
  o_double = i_double + 1.1;
  o_enum = i_enum;
}

void DBusTestServer::test_basic_2(int       i_int,
                                  uint8_t   i_uint8,
                                  int16_t   i_int16,
                                  uint16_t  i_uint16,
                                  int32_t   i_int32,
                                  uint32_t  i_uint32,
                                  int64_t   i_int64,
                                  uint64_t  i_uint64,
                                  const string    &i_string,
                                  bool      i_bool,
                                  double    i_double,
                                  DBusTestData::Enum      i_enum,
                                  int       *o_int,
                                  uint8_t   *o_uint8,
                                  int16_t   *o_int16,
                                  uint16_t  *o_uint16,
                                  int32_t   *o_int32,
                                  uint32_t  *o_uint32,
                                  int64_t   *o_int64,
                                  uint64_t  *o_uint64,
                                  string    *o_string,
                                  bool      *o_bool,
                                  double    *o_double,
                                  DBusTestData::Enum      *o_enum
                                  )
{
  *o_int = i_int + 1;
  *o_uint8 = i_uint8 + 2;
  *o_int16 = i_int16 + 3;
  *o_uint16 = i_uint16 + 4;
  *o_int32 = i_int32 + 5;
  *o_uint32 = i_uint32 + 6;
  *o_int64 = i_int64 + 7;
  *o_uint64 = i_uint64 + 8;
  *o_string = i_string + " World";
  *o_bool = !i_bool;
  *o_double = i_double + 1.1;
  *o_enum = i_enum;
}

void
DBusTestServer::test_struct_1(const DBusTestData::StructWithAllBasicTypes &i_struct,
                              DBusTestData::StructWithAllBasicTypes &o_struct)
{
  o_struct.m_int = i_struct.m_int + 1;
  o_struct.m_uint8 = i_struct.m_uint8 + 2;
  o_struct.m_int16 = i_struct.m_int16 + 3;
  o_struct.m_uint16 = i_struct.m_uint16 + 4;
  o_struct.m_int32 = i_struct.m_int32 + 5;
  o_struct.m_uint32 = i_struct.m_uint32 + 6;
  o_struct.m_int64 = i_struct.m_int64 + 7;
  o_struct.m_uint64 = i_struct.m_uint64 + 8;
  o_struct.m_string = i_struct.m_string + " World";
  o_struct.m_bool = !i_struct.m_bool;
  o_struct.m_double = i_struct.m_double + 1.1;
  o_struct.m_enum = i_struct.m_enum;
}

void
DBusTestServer::test_struct_2(DBusTestData::StructWithAllBasicTypes i_struct,
                              DBusTestData::StructWithAllBasicTypes *o_struct)
{
  o_struct->m_int = i_struct.m_int + 1;
  o_struct->m_uint8 = i_struct.m_uint8 + 2;
  o_struct->m_int16 = i_struct.m_int16 + 3;
  o_struct->m_uint16 = i_struct.m_uint16 + 4;
  o_struct->m_int32 = i_struct.m_int32 + 5;
  o_struct->m_uint32 = i_struct.m_uint32 + 6;
  o_struct->m_int64 = i_struct.m_int64 + 7;
  o_struct->m_uint64 = i_struct.m_uint64 + 8;
  o_struct->m_string = i_struct.m_string + " World";
  o_struct->m_bool = !i_struct.m_bool;
  o_struct->m_double = i_struct.m_double + 1.1;
  o_struct->m_enum = i_struct.m_enum;
}

void
DBusTestServer::test_list_of_struct(DBusTestData::DataList i_data,
                                    DBusTestData::DataList &o_data)
{
  for (auto &d : i_data)
    {
      d.m_data += 76;
      o_data.push_back(d);
    }
}

void DBusTestServer::test_map_of_struct(DBusTestData::DataMap i_data,
                                        DBusTestData::DataMap &o_data)
{
  for (auto &d : i_data)
    {
      o_data[d.first] = d.second;
      o_data[d.first].m_data += 65;
    }
}
