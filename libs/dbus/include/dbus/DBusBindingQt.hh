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

#include <QtDBus/QtDBus>

#include "dbus/DBusBinding.hh"
#include "dbus/IDBus.hh"

namespace workrave
{
  namespace dbus
  {
    class DBus;

    class IDBusPrivateQt
    {
    public:
      typedef std::shared_ptr<IDBusPrivateQt> Ptr;

      virtual ~IDBusPrivateQt()
      {
      }

      virtual QDBusConnection get_connection() = 0;
    };

    class DBusBindingQt : public DBusBinding
    {
    public:
      explicit DBusBindingQt(IDBus::Ptr dbus);
      ~DBusBindingQt() override = default;

      virtual const char *get_interface_introspect() = 0;
      virtual bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection) = 0;

    protected:
      IDBus::Ptr dbus;
    };

    class DBusMarshallQt
    {
    public:
      void get_int(const QVariant &variant, int &value);
      void get_uint8(const QVariant &variant, uint8_t &value);
      void get_uint16(const QVariant &variant, uint16_t &value);
      void get_int16(const QVariant &variant, int16_t &value);
      void get_uint32(const QVariant &variant, uint32_t &value);
      void get_int32(const QVariant &variant, int32_t &value);
      void get_uint64(const QVariant &variant, uint64_t &value);
      void get_int64(const QVariant &variant, int64_t &value);
      void get_bool(const QVariant &variant, bool &value);
      void get_double(const QVariant &variant, double &value);
      void get_string(const QVariant &variant, std::string &value);
      void get_string(const QVariant &variant, QString &value);

      QVariant put_uint8(const uint8_t &value);
      QVariant put_int(const int &value);
      QVariant put_uint16(const uint16_t &value);
      QVariant put_int16(const int16_t &value);
      QVariant put_uint32(const uint32_t &value);
      QVariant put_int32(const int32_t &value);
      QVariant put_uint64(const uint64_t &value);
      QVariant put_int64(const int64_t &value);
      QVariant put_bool(const bool &value);
      QVariant put_double(const double &value);
      QVariant put_string(const std::string &value);
      QVariant put_string(const QString &value);
    };
  } // namespace dbus
} // namespace workrave

#endif // WORKRAVE_DBUS_DBUSBINDINGQT_HH
