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

#include "DBusQt.hh"
#include "dbus/DBusBindingQt.hh"
#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::dbus;

DBusBindingQt::DBusBindingQt(std::shared_ptr<IDBus> dbus)
  : dbus(dbus)
{
}

// // Template to map a QMetaType to a C++ type

// template<>
// uint8_t
// checked_value<uint8_t>(const QVariant &variant)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::UChar)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(variant).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   return variant.value<uint8_t>();
// }

// template<>
// int
// checked_value<int>(const QVariant &variant)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Int)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(variant).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   return variant.value<int>();
// }

// void
// DBusMarshallQt::get_int(const QVariant &variant, int &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Int)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<int>();
// }

// void
// DBusMarshallQt::get_uint16(const QVariant &variant, uint16_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::UShort)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<uint16_t>();
// }

// void
// DBusMarshallQt::get_int16(const QVariant &variant, int16_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Short)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<int16_t>();
// }

// void
// DBusMarshallQt::get_uint32(const QVariant &variant, uint32_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::UInt)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<uint32_t>();
// }

// void
// DBusMarshallQt::get_int32(const QVariant &variant, int32_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Int)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<int32_t>();
// }

// void
// DBusMarshallQt::get_uint64(const QVariant &variant, uint64_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::ULongLong)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<uint64_t>();
// }

// void
// DBusMarshallQt::get_int64(const QVariant &variant, int64_t &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::LongLong)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<int64_t>();
// }

// void
// DBusMarshallQt::get_bool(const QVariant &variant, bool &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Bool)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<bool>();
// }

// void
// DBusMarshallQt::get_double(const QVariant &variant, double &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::Double)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<double>();
// }

// void
// DBusMarshallQt::get_string(const QVariant &variant, std::string &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::QString)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(QString()).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<QString>().toStdString();
// }

// void
// DBusMarshallQt::get_string(const QVariant &variant, QString &value)
// {
//   if (static_cast<QMetaType::Type>(variant.typeId()) != QMetaType::QString)
//     {
//       throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
//                                   << expected_type_info(QVariant::fromValue(value).typeName())
//                                   << actual_type_info(variant.typeName());
//     }
//   value = variant.value<QString>();
// }

// QVariant
// DBusMarshallQt::put_int(const int &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_uint8(const uint8_t &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_uint16(const uint16_t &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_int16(const int16_t &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_uint32(const uint32_t &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_int32(const int32_t &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_uint64(const uint64_t &value)
// {
//   return QVariant(static_cast<qulonglong>(value));
// }

// QVariant
// DBusMarshallQt::put_int64(const int64_t &value)
// {
//   return QVariant(static_cast<qlonglong>(value));
// }

// QVariant
// DBusMarshallQt::put_double(const double &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_bool(const bool &value)
// {
//   return QVariant(value);
// }

// QVariant
// DBusMarshallQt::put_string(const std::string &value)
// {
//   return QVariant(QString::fromStdString(value));
// }

// QVariant
// DBusMarshallQt::put_string(const QString &value)
// {
//   return QVariant(value);
// }
