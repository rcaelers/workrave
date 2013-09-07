// Test.hh
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

#ifndef DBUSTESTDATA_HH
#define DBUSTESTDATA_HH

#include <string>
#include <set>

#include <QDBusArgument>
#include <QMetaType>

class DBusTestData
{
public:
  enum Enum
    {
      ONE, TWO, THREE, FOUR, FIVE,
    };

  struct StructWithAllBasicTypes
  {
    int         m_int;
    uint8_t     m_uint8;
    int16_t     m_int16;
    uint16_t    m_uint16;
    int32_t     m_int32;
    uint32_t    m_uint32;
    int64_t     m_int64;
    uint64_t    m_uint64;
    std::string m_string;
    bool        m_bool;
    double      m_double;
    Enum        m_enum;
  };
  
  typedef std::list<StructWithAllBasicTypes> ListOfStructWithAllBasicTypes;
  typedef std::map<std::string, StructWithAllBasicTypes> MapOfStructWithAllBasicTypes;

  struct Data
  {
    Data() : m_key(0), m_data(0) {}
    Data(int k, int d) : m_key(k), m_data(d) {}


    bool operator==(const Data &other) const
    {
      return m_key == other.m_key && m_data == other.m_data;
    }
    
    int              m_key;
    int              m_data;
  };
  
  typedef std::map<std::string, Data> DataMap;
  typedef std::list<Data> DataList;

  typedef std::map<std::string, std::string> StringMap;
  typedef std::list<std::string> StringList;
  
  static std::string enum_to_str(DBusTestData::Enum value)
  {
    std::string ret;

    switch (value)
      {
      case DBusTestData::ONE:
        ret = "one";
        break;
      case DBusTestData::TWO:
        ret = "two";
        break;
      case DBusTestData::THREE:
        ret = "three";
        break;
      case DBusTestData::FOUR:
        ret = "four";
        break;
      case DBusTestData::FIVE:
        ret = "five";
        break;
      }

    return ret;
  }

  static Enum str_to_enum(const std::string &value)
  {
    Enum ret = DBusTestData::ONE;
    if ("one" == value)
      {
        ret = DBusTestData::ONE;
      }
    else if ("two" == value)
      {
        ret = DBusTestData::TWO;
      }
    else if ("three" == value)
      {
        ret = DBusTestData::THREE;
      }
    else if ("four" == value)
      {
        ret = DBusTestData::FOUR;
      }
    else if ("five" == value)
      {
        ret = DBusTestData::FIVE;
      }

    return ret;
  }

  
  DBusTestData();
  virtual ~DBusTestData();
};

#ifdef DBUS_BACKEND_QT5
Q_DECLARE_METATYPE(DBusTestData::StructWithAllBasicTypes)
Q_DECLARE_METATYPE(DBusTestData::Data)

QDBusArgument &operator<<(QDBusArgument &argument, const DBusTestData::StructWithAllBasicTypes& message);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusTestData::StructWithAllBasicTypes &message);

QDBusArgument &operator<<(QDBusArgument &argument, const DBusTestData::Data& message);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusTestData::Data &message);
#endif


#endif // DBUSTESTDATA_HH
