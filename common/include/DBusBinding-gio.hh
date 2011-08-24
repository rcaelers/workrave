// DBusBinding-gio.hh --- DBUS interface
//
// Copyright (C) 2007, 2008, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef DBUSBINDINGGIO_HH
#define DBUSBINDINGGIO_HH

#include <string>
#include <gio/gio.h>

namespace workrave
{
  class DBus;

  class DBusBaseTypes
  {
  public:
    void get_uint8(GVariant *v, guint8 *value);
    void get_uint16(GVariant *v, guint16 *value);
    void get_int16(GVariant *v, gint16 *value);
    void get_uint32(GVariant *v, guint32 *value);
    void get_int32(GVariant *v, gint32 *value);
    void get_uint64(GVariant *v, guint64 *value);
    void get_int64(GVariant *v, gint64 *value);
    void get_bool(GVariant *v, bool *value);
    void get_double(GVariant *v, double *value);
    void get_string(GVariant *v, std::string *value);

    GVariant *put_uint8(const guint8 *value);
    GVariant *put_uint16(const guint16 *value);
    GVariant *put_int16(const gint16 *value);
    GVariant *put_uint32(const guint32 *value);
    GVariant *put_int32(const gint32 *value);
    GVariant *put_uint64(const guint64 *value);
    GVariant *put_int64(const gint64 *value);
    GVariant *put_bool(const bool *value);
    GVariant *put_double(const double *value);
    GVariant *put_string(const std::string *value);
  };

  class DBusBindingBase : public DBusBaseTypes
  {
  public:
    DBusBindingBase(DBus *dbus);
    virtual ~DBusBindingBase();

    virtual const char *get_interface_introspect() = 0;
    virtual void call(const std::string &method, void *object, GDBusMethodInvocation *invocation, const std::string &sender, GVariant *inargs) = 0;

  protected:
    DBus *dbus;
  };
}

#endif // DBUSBINDINGGIO_HH
