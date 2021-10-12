// Copyright (C) 2007, 2008, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_DBUS_DBUSBINDINGFREEDESKTOP_HH
#define WORKRAVE_DBUS_DBUSBINDINGFREEDESKTOP_HH

#include <dbus/dbus.h>
#include <glib.h>
#include <string>

#include "dbus/DBusBinding.hh"
#include "dbus/IDBus.hh"

namespace workrave
{
  namespace dbus
  {
    class IDBusPrivateFreeDesktop
    {
    public:
      typedef std::shared_ptr<IDBusPrivateFreeDesktop> Ptr;

      virtual ~IDBusPrivateFreeDesktop()
      {
      }

      virtual void send(DBusMessage *msg) const = 0;
    };

    struct DBusIntrospectArg
    {
      const char *name;
      const char *type;
      const char *direction;
    };

    struct DBusIntrospect
    {
      const char *name;
      const char *signature;
    };

    class DBusBindingFreeDesktop : public DBusBinding
    {
    public:
      explicit DBusBindingFreeDesktop(IDBus::Ptr dbus);
      virtual ~DBusBindingFreeDesktop();

      virtual DBusIntrospect *get_method_introspect() = 0;
      virtual DBusIntrospect *get_signal_introspect() = 0;

      DBusMessage *call(const std::string &method, void *object, DBusMessage *message);

    protected:
      virtual DBusMessage *call(int method, void *object, DBusMessage *message) = 0;
      void send(DBusMessage *msg);

      void get_uint8(DBusMessageIter *it, uint8_t *value);
      void get_uint16(DBusMessageIter *it, uint16_t *value);
      void get_int16(DBusMessageIter *it, int16_t *value);
      void get_uint32(DBusMessageIter *it, uint32_t *value);
      void get_int32(DBusMessageIter *it, int32_t *value);
      void get_uint64(DBusMessageIter *it, uint64_t *value);
      void get_int64(DBusMessageIter *it, int64_t *value);
      void get_bool(DBusMessageIter *it, bool *value);
      void get_double(DBusMessageIter *it, double *value);
      void get_string(DBusMessageIter *it, std::string *value);

      void put_uint8(DBusMessageIter *it, const uint8_t *value);
      void put_uint16(DBusMessageIter *it, const uint16_t *value);
      void put_int16(DBusMessageIter *it, const int16_t *value);
      void put_uint32(DBusMessageIter *it, const uint32_t *value);
      void put_int32(DBusMessageIter *it, const int32_t *value);
      void put_uint64(DBusMessageIter *it, const uint64_t *value);
      void put_int64(DBusMessageIter *it, const int64_t *value);
      void put_bool(DBusMessageIter *it, const bool *value);
      void put_double(DBusMessageIter *it, const double *value);
      void put_string(DBusMessageIter *it, const std::string *value);

      IDBus::Ptr dbus;
    };
  } // namespace dbus
} // namespace workrave

#endif // WORKRAVE_DBUS_DBUSBINDINGFREEDESKTOP_HH
