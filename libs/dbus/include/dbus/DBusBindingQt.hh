// Copyright (C) 2013, 2025 Rob Caelers <robc@krandor.nl>
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

#include <qtypes.h>
#include <qvariant.h>
#include <spdlog/spdlog.h>
#include <string>
#include <cstdint>
#include <boost/lexical_cast.hpp>

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
  struct DBusCpptoQt
  {
    using Type = T;
    static QMetaType::Type qt_type()
    {
      return static_cast<QMetaType::Type>(QMetaType::fromType<T>().id());
    }
  };

  template<>
  struct DBusCpptoQt<std::string>
  {
    using Type = QString;
    static QMetaType::Type qt_type()
    {
      return QMetaType::QString;
    }
  };

  template<>
  struct DBusCpptoQt<int64_t>
  {
    using Type = qlonglong;
    static QMetaType::Type qt_type()
    {
      return QMetaType::LongLong;
    }
  };

  template<>
  struct DBusCpptoQt<uint64_t>
  {
    using Type = qulonglong;
    static QMetaType::Type qt_type()
    {
      return QMetaType::ULongLong;
    }
  };

  template<typename T>
  struct DBusMarshall
  {
    static T convert(const QVariant &variant)
    {
      if (variant.typeId() != DBusCpptoQt<T>::qt_type())
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info(QMetaType::fromType<T>().name());
        }
      return variant.value<T>();
    }
    static QVariant convert(const T &value)
    {
      return QVariant::fromValue(value);
    }
  };

  template<>
  struct DBusMarshall<std::string>
  {
    static std::string convert(const QVariant &variant)
    {
      if (variant.typeId() != DBusCpptoQt<std::string>::qt_type())
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info("string");
        }
      return variant.toString().toStdString();
    }
    static QVariant convert(const std::string &value)
    {
      return QString::fromStdString(value);
    }
  };

  template<>
  struct DBusMarshall<int64_t>
  {
    static int64_t convert(const QVariant &variant)
    {
      if (variant.typeId() != DBusCpptoQt<int64_t>::qt_type())
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info("int64_t");
        }
      return static_cast<int64_t>(variant.value<qlonglong>());
    }
    static QVariant convert(int64_t value)
    {
      return static_cast<qlonglong>(value);
    }
  };

  template<>
  struct DBusMarshall<uint64_t>
  {
    static uint64_t convert(const QVariant &variant)
    {
      if (variant.typeId() != DBusCpptoQt<uint64_t>::qt_type())
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info("uint64_t");
        }
      return static_cast<uint64_t>(variant.value<qulonglong>());
    }
    static QVariant convert(uint64_t value)
    {
      return static_cast<qulonglong>(value);
    }
  };

  template<typename K, typename V>
  struct DBusMarshall<std::map<K, V>>
  {
    using K_qt = DBusCpptoQt<K>::Type;
    using V_qt = DBusCpptoQt<V>::Type;

    static std::map<K, V> convert(const QVariant &variant)
    {
      const auto arg = variant.value<QDBusArgument>();
      std::map<K, V> result;

      if (arg.currentType() != QDBusArgument::MapType)
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info("std::map");
        }

      arg.beginMap();

      while (!arg.atEnd())
        {
          K key;
          V value;

          arg.beginMapEntry();
          try
            {
              key = DBusMarshall<K>::convert(arg.asVariant());
              value = DBusMarshall<V>::convert(arg.asVariant());
            }
          catch (const DBusRemoteException &e)
            {
              e << field_info(key);
              throw;
            }
          arg.endMapEntry();

          (result)[key] = value;
        }

      arg.endMap();
      return result;
    }
    static QVariant convert(const std::map<K, V> &value)
    {
      QDBusArgument arg;

      arg.beginMap(qMetaTypeId<K_qt>(), qMetaTypeId<V_qt>());

      for (auto &it: value)
        {
          arg.beginMapEntry();
          arg.appendVariant(DBusMarshall<K>::convert(it.first));
          arg.appendVariant(DBusMarshall<V>::convert(it.second));
          arg.endMapEntry();
        }
      arg.endMap();

      return QVariant::fromValue(arg);
    }
  };

  template<typename V>
  struct DBusMarshall<std::list<V>>
  {
    using V_qt = DBusCpptoQt<V>::Type;

    static std::list<V> convert(const QVariant &variant)
    {
      const auto arg = variant.value<QDBusArgument>();
      std::list<V> result;

      if (arg.currentType() != QDBusArgument::ArrayType)
        {
          throw DBusRemoteException() << message_info("Incorrect type") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                      << expected_type_info("std::list");
        }

      arg.beginArray();
      while (!arg.atEnd())
        {
          V value;
          try
            {
              value = DBusMarshall<V>::convert(arg.asVariant());
            }
          catch (const DBusRemoteException &e)
            {
              e << field_info(std::string("[") + boost::lexical_cast<std::string>(result.size()) + "]");
              throw;
            }

          result.push_back(value);
        }
      arg.endArray();
      return result;
    }
    static QVariant convert(const std::list<V> &value)
    {
      QDBusArgument arg;

      arg.beginArray(qMetaTypeId<V_qt>());
      for (auto &it: value)
        {
          arg.appendVariant(DBusMarshall<V>::convert(it));
        }
      arg.endArray();

      return QVariant::fromValue(arg);
    }
  };

} // namespace workrave::dbus

#endif // WORKRAVE_DBUS_DBUSBINDINGQT_HH
