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

#ifndef DBUSTESTDATAMETA_HH
#define DBUSTESTDATAMETA_HH

#include "DBusTestData.hh"

#if defined(DBUS_BACKEND_QT)
#  include <QDBusArgument>
#  include <QMetaType>
#endif

Q_DECLARE_METATYPE(DBusTestData::StructWithAllBasicTypes)
Q_DECLARE_METATYPE(DBusTestData::StructWithAllBasicTypesReorder)
Q_DECLARE_METATYPE(DBusTestData::Data)

QDBusArgument &
operator<<(QDBusArgument &argument, const DBusTestData::StructWithAllBasicTypes &message)
{
  argument.beginStructure();

  argument << message.m_int;
  argument << message.m_uint8;
  argument << message.m_int16;
  argument << message.m_uint16;
  argument << message.m_int32;
  argument << message.m_uint32;
  argument << static_cast<qlonglong>(message.m_int64);
  argument << static_cast<qulonglong>(message.m_uint64);
  argument << QString::fromStdString(message.m_string);
  argument << message.m_bool;
  argument << message.m_double;
  QString e = QString::fromStdString(DBusTestData::enum_to_str(message.m_enum));
  argument << e;

  argument.endStructure();
  return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, DBusTestData::StructWithAllBasicTypes &message)
{
  argument.beginStructure();

  argument >> message.m_int;
  argument >> message.m_uint8;
  argument >> message.m_int16;
  argument >> message.m_uint16;
  argument >> message.m_int32;
  argument >> message.m_uint32;
  qlonglong l;
  argument >> l;
  message.m_int64 = l;
  qlonglong ul;
  argument >> ul;
  message.m_uint64 = ul;
  QString s;
  argument >> s;
  message.m_string = s.toStdString();
  argument >> message.m_bool;
  argument >> message.m_double;
  QString e;
  argument >> e;
  message.m_enum = DBusTestData::str_to_enum(e.toStdString());
  argument.endStructure();

  return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const DBusTestData::StructWithAllBasicTypesReorder &message)
{
  argument.beginStructure();

  argument << message.m_int;
  argument << message.m_uint8;
  argument << message.m_int16;
  argument << message.m_uint16;
  argument << QString::fromStdString(message.m_string);
  argument << message.m_int32;
  argument << message.m_uint32;
  argument << static_cast<qlonglong>(message.m_int64);
  argument << static_cast<qulonglong>(message.m_uint64);
  argument << message.m_bool;
  argument << message.m_double;
  QString e = QString::fromStdString(DBusTestData::enum_to_str(message.m_enum));
  argument << e;

  argument.endStructure();
  return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, DBusTestData::StructWithAllBasicTypesReorder &message)
{
  argument.beginStructure();

  argument >> message.m_int;
  argument >> message.m_uint8;
  argument >> message.m_int16;
  argument >> message.m_uint16;
  QString s;
  argument >> s;
  message.m_string = s.toStdString();
  argument >> message.m_int32;
  argument >> message.m_uint32;
  qlonglong l;
  argument >> l;
  message.m_int64 = l;
  qlonglong ul;
  argument >> ul;
  message.m_uint64 = ul;
  argument >> message.m_bool;
  argument >> message.m_double;
  QString e;
  argument >> e;
  message.m_enum = DBusTestData::str_to_enum(e.toStdString());
  argument.endStructure();

  return argument;
}

QDBusArgument &
operator<<(QDBusArgument &argument, const DBusTestData::Data &message)
{
  argument.beginStructure();
  argument << message.m_key;
  argument << message.m_data;
  argument.endStructure();
  return argument;
}

const QDBusArgument &
operator>>(const QDBusArgument &argument, DBusTestData::Data &message)
{
  argument.beginStructure();
  argument >> message.m_key;
  argument >> message.m_data;
  argument.endStructure();
  return argument;
}

#endif // DBUSTESTDATA_HH
