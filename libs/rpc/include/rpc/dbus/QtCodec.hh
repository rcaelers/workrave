// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <type_traits>
#include <utility>

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusSignature>
#include <QtDBus/QDBusUnixFileDescriptor>
#include <QtDBus/QDBusVariant>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/WireTypes.hh"

namespace workrave::rpc::dbus
{
  template<typename T>
  struct QtType
  {
    using Type = T;

    static int id()
    {
      return QMetaType::fromType<T>().id();
    }
  };

  template<>
  struct QtType<std::string>
  {
    using Type = QString;

    static int id()
    {
      return QMetaType::QString;
    }
  };

  template<>
  struct QtType<ObjectPath>
  {
    using Type = QDBusObjectPath;

    static int id()
    {
      return qMetaTypeId<QDBusObjectPath>();
    }
  };

  template<>
  struct QtType<Signature>
  {
    using Type = QDBusSignature;

    static int id()
    {
      return qMetaTypeId<QDBusSignature>();
    }
  };

  template<>
  struct QtType<UnixFd>
  {
    using Type = QDBusUnixFileDescriptor;

    static int id()
    {
      return qMetaTypeId<QDBusUnixFileDescriptor>();
    }
  };

  template<typename Value>
  struct QtType<Variant<Value>>
  {
    using Type = QDBusVariant;

    static int id()
    {
      return qMetaTypeId<QDBusVariant>();
    }
  };

  template<>
  struct QtType<int64_t>
  {
    using Type = qlonglong;

    static int id()
    {
      return QMetaType::LongLong;
    }
  };

  template<>
  struct QtType<uint64_t>
  {
    using Type = qulonglong;

    static int id()
    {
      return QMetaType::ULongLong;
    }
  };

  inline Error qt_type_error(std::string expected)
  {
    return Error(std::string(error_names::invalid_args), "Incorrect DBus argument type; expected " + std::move(expected));
  }

  template<typename T>
  struct QtCodec
  {
    static T decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<T>::id())
        {
          throw qt_type_error(QMetaType::fromType<T>().name());
        }
      return variant.value<T>();
    }

    static QVariant encode(const T &value)
    {
      return QVariant::fromValue(value);
    }

    static void append(QDBusArgument &argument, const T &value)
    {
      argument << value;
    }
  };

  template<>
  struct QtCodec<std::string>
  {
    static std::string decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<std::string>::id())
        {
          throw qt_type_error("string");
        }
      return variant.toString().toStdString();
    }

    static QVariant encode(const std::string &value)
    {
      return QString::fromStdString(value);
    }

    static void append(QDBusArgument &argument, const std::string &value)
    {
      argument << QString::fromStdString(value);
    }
  };

  template<>
  struct QtCodec<ObjectPath>
  {
    static ObjectPath decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<ObjectPath>::id())
        {
          throw qt_type_error("object path");
        }
      return ObjectPath{variant.value<QDBusObjectPath>().path().toStdString()};
    }

    static QVariant encode(const ObjectPath &value)
    {
      QDBusObjectPath path(QString::fromStdString(value.value));
      if (path.path().isEmpty())
        {
          throw Error(std::string(error_names::invalid_args),
                      "Invalid D-Bus object path: " + value.value);
        }
      return QVariant::fromValue(path);
    }

    static void append(QDBusArgument &argument, const ObjectPath &value)
    {
      argument << encode(value).value<QDBusObjectPath>();
    }
  };

  template<>
  struct QtCodec<Signature>
  {
    static Signature decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<Signature>::id())
        {
          throw qt_type_error("signature");
        }
      return Signature{variant.value<QDBusSignature>().signature().toStdString()};
    }

    static QVariant encode(const Signature &value)
    {
      QDBusSignature signature(QString::fromStdString(value.value));
      if (signature.signature().isEmpty() && !value.value.empty())
        {
          throw Error(std::string(error_names::invalid_args),
                      "Invalid D-Bus signature: " + value.value);
        }
      return QVariant::fromValue(signature);
    }

    static void append(QDBusArgument &argument, const Signature &value)
    {
      argument << encode(value).value<QDBusSignature>();
    }
  };

  template<>
  struct QtCodec<UnixFd>
  {
    static UnixFd decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<UnixFd>::id())
        {
          throw qt_type_error("UNIX file descriptor");
        }
      return UnixFd{variant.value<QDBusUnixFileDescriptor>().fileDescriptor()};
    }

    static QVariant encode(UnixFd value)
    {
      return QVariant::fromValue(QDBusUnixFileDescriptor(value.value));
    }

    static void append(QDBusArgument &argument, UnixFd value)
    {
      argument << QDBusUnixFileDescriptor(value.value);
    }
  };

  template<typename Value>
  struct QtCodec<Variant<Value>>
  {
    static Variant<Value> decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<Variant<Value>>::id())
        {
          throw qt_type_error("variant");
        }
      return Variant<Value>{QtCodec<Value>::decode(variant.value<QDBusVariant>().variant())};
    }

    static QVariant encode(const Variant<Value> &value)
    {
      return QVariant::fromValue(QDBusVariant(QtCodec<Value>::encode(value.value)));
    }

    static void append(QDBusArgument &argument, const Variant<Value> &value)
    {
      argument << QDBusVariant(QtCodec<Value>::encode(value.value));
    }
  };

  template<>
  struct QtCodec<int64_t>
  {
    static int64_t decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<int64_t>::id())
        {
          throw qt_type_error("int64");
        }
      return static_cast<int64_t>(variant.value<qlonglong>());
    }

    static QVariant encode(int64_t value)
    {
      return static_cast<qlonglong>(value);
    }

    static void append(QDBusArgument &argument, int64_t value)
    {
      argument << static_cast<qlonglong>(value);
    }
  };

  template<>
  struct QtCodec<uint64_t>
  {
    static uint64_t decode(const QVariant &variant)
    {
      if (variant.typeId() != QtType<uint64_t>::id())
        {
          throw qt_type_error("uint64");
        }
      return static_cast<uint64_t>(variant.value<qulonglong>());
    }

    static QVariant encode(uint64_t value)
    {
      return static_cast<qulonglong>(value);
    }

    static void append(QDBusArgument &argument, uint64_t value)
    {
      argument << static_cast<qulonglong>(value);
    }
  };

  template<typename Key, typename Value>
  struct QtCodec<std::map<Key, Value>>
  {
    using QtKey = typename QtType<Key>::Type;
    using QtValue = typename QtType<Value>::Type;

    static std::map<Key, Value> decode(const QVariant &variant)
    {
      const auto argument = variant.value<QDBusArgument>();
      if (argument.currentType() != QDBusArgument::MapType)
        {
          throw qt_type_error("map");
        }

      std::map<Key, Value> result;
      argument.beginMap();
      while (!argument.atEnd())
        {
          argument.beginMapEntry();
          auto key = QtCodec<Key>::decode(argument.asVariant());
          auto value = QtCodec<Value>::decode(argument.asVariant());
          argument.endMapEntry();
          result.emplace(std::move(key), std::move(value));
        }
      argument.endMap();
      return result;
    }

    static void append(QDBusArgument &argument, const std::map<Key, Value> &value)
    {
      argument.beginMap(qMetaTypeId<QtKey>(), qMetaTypeId<QtValue>());
      for (const auto &[key, item]: value)
        {
          argument.beginMapEntry();
          QtCodec<Key>::append(argument, key);
          QtCodec<Value>::append(argument, item);
          argument.endMapEntry();
        }
      argument.endMap();
    }

    static QVariant encode(const std::map<Key, Value> &value)
    {
      QDBusArgument argument;
      append(argument, value);
      return QVariant::fromValue(argument);
    }
  };

  template<typename Value>
  struct QtCodec<std::list<Value>>
  {
    using QtValue = typename QtType<Value>::Type;

    static std::list<Value> decode(const QVariant &variant)
    {
      const auto argument = variant.value<QDBusArgument>();
      if (argument.currentType() != QDBusArgument::ArrayType)
        {
          throw qt_type_error("array");
        }

      std::list<Value> result;
      argument.beginArray();
      while (!argument.atEnd())
        {
          result.push_back(QtCodec<Value>::decode(argument.asVariant()));
        }
      argument.endArray();
      return result;
    }

    static void append(QDBusArgument &argument, const std::list<Value> &value)
    {
      argument.beginArray(qMetaTypeId<QtValue>());
      for (const auto &item: value)
        {
          QtCodec<Value>::append(argument, item);
        }
      argument.endArray();
    }

    static QVariant encode(const std::list<Value> &value)
    {
      QDBusArgument argument;
      append(argument, value);
      return QVariant::fromValue(argument);
    }
  };
}
