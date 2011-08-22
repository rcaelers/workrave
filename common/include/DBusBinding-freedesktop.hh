// DBusBinding.hh --- DBUS interface
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

#ifndef DBUSBINDINGFREEDESKTOP_HH
#define DBUSBINDINGFREEDESKTOP_HH

#include <dbus/dbus.h>
#include <glib.h>
#include <string>

namespace workrave
{
  class DBus;

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

  class DBusBaseTypes
  {
  public:
    void get_uint8(DBusMessageIter *it, guint8 *value);
    void get_uint16(DBusMessageIter *it, guint16 *value);
    void get_int16(DBusMessageIter *it, gint16 *value);
    void get_uint32(DBusMessageIter *it, guint32 *value);
    void get_int32(DBusMessageIter *it, gint32 *value);
    void get_uint64(DBusMessageIter *it, guint64 *value);
    void get_int64(DBusMessageIter *it, gint64 *value);
    void get_bool(DBusMessageIter *it, bool *value);
    void get_double(DBusMessageIter *it, double *value);
    void get_string(DBusMessageIter *it, std::string *value);

    void put_uint8(DBusMessageIter *it, const guint8 *value);
    void put_uint16(DBusMessageIter *it, const guint16 *value);
    void put_int16(DBusMessageIter *it, const gint16 *value);
    void put_uint32(DBusMessageIter *it, const guint32 *value);
    void put_int32(DBusMessageIter *it, const gint32 *value);
    void put_uint64(DBusMessageIter *it, const guint64 *value);
    void put_int64(DBusMessageIter *it, const gint64 *value);
    void put_bool(DBusMessageIter *it, const bool *value);
    void put_double(DBusMessageIter *it, const double *value);
    void put_string(DBusMessageIter *it, const std::string *value);
  };

  class DBusBindingBase : public DBusBaseTypes
  {
  public:
    DBusBindingBase(DBus *dbus);
    virtual ~DBusBindingBase();

    virtual DBusIntrospect *get_method_introspect() = 0;
    virtual DBusIntrospect *get_signal_introspect() = 0;

    DBusMessage *call(const std::string &method, void *object, DBusMessage *message);

  protected:
    virtual DBusMessage *call(int method, void *object, DBusMessage *message) = 0;
    void send(DBusMessage *msg);

    DBus *dbus;
  };
}

#endif // DBUSBINDINGFREEDESKTOP_HH
