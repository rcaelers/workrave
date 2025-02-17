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

#ifndef WORKRAVE_DBUS_DBUSBINDINGQT_HH
#define WORKRAVE_DBUS_DBUSBINDINGQT_HH

#include <string>
#include <cstdint>

#include <QtDBus/QtDBus>

#include "dbus/DBusBinding.hh"
#include "dbus/IDBus.hh"

namespace workrave::dbus
{
  class DBus;

  class IDBusPrivateQt
  {
  public:
    using Ptr = std::shared_ptr<IDBusPrivateQt>;
    virtual ~IDBusPrivateQt() = default;
    IDBusPrivateQt() = default;
    IDBusPrivateQt(const IDBusPrivateQt &) = delete;
    IDBusPrivateQt &operator=(const IDBusPrivateQt &) = delete;
    IDBusPrivateQt(IDBusPrivateQt &&) = delete;
    IDBusPrivateQt &operator=(IDBusPrivateQt &&) = delete;

    virtual QDBusConnection get_connection() = 0;
  };

  class DBusBindingQt : public DBusBinding
  {
  public:
    explicit DBusBindingQt(std::shared_ptr<IDBus> dbus);
    ~DBusBindingQt() override = default;

    virtual std::string_view get_interface_introspect() = 0;
    virtual bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection) = 0;

  protected:
    std::shared_ptr<IDBus> dbus;
  };

  template<typename T>
  struct QtCppTypeMap
  {
    static QMetaType::Type qtType()
    {
      return static_cast<QMetaType::Type>(QMetaType::fromType<T>().id());
    }
    static const char *qtName()
    {
      // return QMetaType::typeName(qtType());
      return QMetaType::fromType<T>().name();
    }
    static T convert(const QVariant &variant)
    {
      return variant.value<T>();
    }
    static QVariant convert(const T &value)
    {
      return QVariant::fromValue(value);
    }
  };

  template<>
  struct QtCppTypeMap<std::string>
  {
    static QMetaType::Type qtType()
    {
      return QMetaType::QString;
    }
    static const char *qtName()
    {
      return "QString";
    }
    static std::string convert(const QVariant &variant)
    {
      return variant.toString().toStdString();
    }
    static QVariant convert(const std::string &value)
    {
      return QString::fromStdString(value);
    }
  };
  ;

  template<>
  struct QtCppTypeMap<int64_t>
  {
    static QMetaType::Type qtType()
    {
      return QMetaType::LongLong;
    }
    static const char *qtName()
    {
      return "qlonglong";
    }
    static int64_t convert(const QVariant &variant)
    {
      return static_cast<int64_t>(variant.value<qlonglong>());
    }
    static QVariant convert(int64_t value)
    {
      return static_cast<qlonglong>(value);
    }
  };

  template<>
  struct QtCppTypeMap<uint64_t>
  {
    static QMetaType::Type qtType()
    {
      return QMetaType::ULongLong;
    }
    static const char *qtName()
    {
      return "qulonglong";
    }
    static uint64_t convert(const QVariant &variant)
    {
      return static_cast<uint64_t>(variant.value<qulonglong>());
    }
    static QVariant convert(int64_t value)
    {
      return static_cast<qulonglong>(value);
    }
  };

  template<typename T>
  T variant_to_value(const QVariant &variant)
  {
    if (static_cast<QMetaType::Type>(variant.typeId()) != QtCppTypeMap<T>::qtType())
      {
        throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                    << expected_type_info(QtCppTypeMap<T>::qtName()) << actual_type_info(variant.typeName());
      }
    return QtCppTypeMap<T>::convert(variant);
  }

  template<typename T>
  QVariant value_to_variant(const T &value)
  {
    if (static_cast<QMetaType::Type>(QMetaType::fromType<T>().id()) != QtCppTypeMap<T>::qtType())
      {
        throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                    << expected_type_info(QtCppTypeMap<T>::qtName())
                                    << actual_type_info(QMetaType::fromType<T>().name());
      }
    QVariant variant;
    variant.setValue(QtCppTypeMap<T>::convert(value));
    return variant;
  }
} // namespace workrave::dbus

#endif // WORKRAVE_DBUS_DBUSBINDINGQT_HH
