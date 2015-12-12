// DBusBinding-gio.c
//
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
#include "config.h"
#endif

#include "debug.hh"

#include "DBusQt5.hh"
#include "dbus/DBusBindingQt5.hh"
#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::dbus;


DBusBindingQt5::DBusBindingQt5(IDBus::Ptr dbus)
  :dbus(dbus)
{
}

DBusBindingQt5::~DBusBindingQt5()
{
}

void
DBusMarshallQt5::get_uint8(const QVariant &variant, uint8_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::UChar)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<uint8_t>();
}


void
DBusMarshallQt5::get_int(const QVariant &variant, int &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::Int)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<int>();
}

void
DBusMarshallQt5::get_uint16(const QVariant &variant, uint16_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::UShort)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<uint16_t>();
}

void
DBusMarshallQt5::get_int16(const QVariant &variant, int16_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::Short)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<int16_t>();
}

void
DBusMarshallQt5::get_uint32(const QVariant &variant, uint32_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::UInt)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<uint32_t>();
}

void
DBusMarshallQt5::get_int32(const QVariant &variant, int32_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::Int)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<int32_t>();
}

void
DBusMarshallQt5::get_uint64(const QVariant &variant, uint64_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::ULongLong)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<uint64_t>();
}

void
DBusMarshallQt5::get_int64(const QVariant &variant, int64_t &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::LongLong)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<int64_t>();
}

void
DBusMarshallQt5::get_bool(const QVariant &variant, bool &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::Bool)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<bool>();
}

void
DBusMarshallQt5::get_double(const QVariant &variant, double &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::Double)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<double>();
}

void
DBusMarshallQt5::get_string(const QVariant &variant, std::string &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::QString)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(QString()).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<QString>().toStdString();
}

void
DBusMarshallQt5::get_string(const QVariant &variant, QString &value)
{
  if (static_cast<QMetaType::Type>(variant.type()) != QMetaType::QString)
    {
      throw DBusRemoteException()
        << message_info("Type error")
        << error_code_info(DBUS_ERROR_INVALID_ARGS)
        << expected_type_info(QVariant::fromValue(value).typeName())
        << actual_type_info(variant.typeName());
    }
  value = variant.value<QString>();
}

QVariant
DBusMarshallQt5::put_int(const int &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_uint8(const uint8_t &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_uint16(const uint16_t &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_int16(const int16_t &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_uint32(const uint32_t &value)
{
  return QVariant(value);
}


QVariant
DBusMarshallQt5::put_int32(const int32_t &value)
{
  return QVariant(value);
}


QVariant
DBusMarshallQt5::put_uint64(const uint64_t &value)
{
  return QVariant(static_cast<qulonglong>(value));
}

QVariant
DBusMarshallQt5::put_int64(const int64_t &value)
{
  return QVariant(static_cast<qlonglong>(value));
}


QVariant
DBusMarshallQt5::put_double(const double &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_bool(const bool &value)
{
  return QVariant(value);
}

QVariant
DBusMarshallQt5::put_string(const std::string &value)
{
  return QVariant(QString::fromStdString(value));
}

QVariant
DBusMarshallQt5::put_string(const QString &value)
{
  return QVariant(value);
}
