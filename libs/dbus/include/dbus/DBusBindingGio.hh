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

#ifndef WORKRAVE_DBUS_DBUSBINDINGGIO_HH
#define WORKRAVE_DBUS_DBUSBINDINGGIO_HH

#include <string>
#include <gio/gio.h>

#include "dbus/DBusBinding.hh"
#include "dbus/IDBus.hh"

namespace workrave
{
  namespace dbus
  {
    class IDBusPrivateGio
    {
    public:
      using Ptr = std::shared_ptr<IDBusPrivateGio>;

      virtual ~IDBusPrivateGio() = default;

      virtual GDBusConnection *get_connection() const = 0;
    };

    class DBusBindingGio : public DBusBinding
    {
    public:
      explicit DBusBindingGio(IDBus::Ptr dbus);
      ~DBusBindingGio() override = default;

      virtual const char *get_interface_introspect() = 0;
      virtual void call(const std::string &method,
                        void *object,
                        GDBusMethodInvocation *invocation,
                        const std::string &sender,
                        GVariant *inargs) = 0;

    protected:
      IDBus::Ptr dbus;
    };

    class DBusMarshallGio
    {
    public:
      void get_int(GVariant *v, int *value);
      void get_uint8(GVariant *v, uint8_t *value);
      void get_uint16(GVariant *v, uint16_t *value);
      void get_int16(GVariant *v, int16_t *value);
      void get_uint32(GVariant *v, uint32_t *value);
      void get_int32(GVariant *v, int32_t *value);
      void get_uint64(GVariant *v, uint64_t *value);
      void get_int64(GVariant *v, int64_t *value);
      void get_bool(GVariant *v, bool *value);
      void get_double(GVariant *v, double *value);
      void get_string(GVariant *v, std::string *value);

      GVariant *put_int(const int *value);
      GVariant *put_uint8(const uint8_t *value);
      GVariant *put_uint16(const uint16_t *value);
      GVariant *put_int16(const int16_t *value);
      GVariant *put_uint32(const uint32_t *value);
      GVariant *put_int32(const int32_t *value);
      GVariant *put_uint64(const uint64_t *value);
      GVariant *put_int64(const int64_t *value);
      GVariant *put_bool(const bool *value);
      GVariant *put_double(const double *value);
      GVariant *put_string(const std::string *value);
    };
  } // namespace dbus
} // namespace workrave

#endif // WORKRAVE_DBUS_DBUSBINDINGGIO_HH
