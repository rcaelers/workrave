// DBusBinding.hh --- DBUS interface
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id: DBus.hh 1380 2007-12-04 21:45:36Z rcaelers $
//

#ifndef DBUSBINDING_HH
#define DBUSBINDING_HH

#include <dbus/dbus.h>
#include <glib.h>
#include <string>

namespace workrave
{
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

  class DBusBindingBase
  {
  public:
    DBusBindingBase();
    virtual ~DBusBindingBase();
  
    virtual DBusIntrospect *get_method_introspect() = 0;
    virtual DBusIntrospect *get_signal_introspect() = 0;

    void init(DBusConnection *connection);
    DBusMessage *call(const std::string &method, void *object, DBusMessage *dbus_message);

  protected:
    virtual DBusMessage *call(int method, void *object, DBusMessage *dbus_message) = 0;
  
    void dbus_get_int(DBusMessageIter *it, int *value);
    void dbus_get_guint32(DBusMessageIter *it, guint32 *value);
    void dbus_get_gint32(DBusMessageIter *it, gint32 *value);
    void dbus_get_guint64(DBusMessageIter *it, guint64 *value);
    void dbus_get_gint64(DBusMessageIter *it, gint64 *value);
    void dbus_get_bool(DBusMessageIter *it, bool *value);
    void dbus_get_double(DBusMessageIter *it, double *value);
    void dbus_get_string(DBusMessageIter *it, std::string *value);

    void dbus_put_int(DBusMessageIter *it, int *value);
    void dbus_put_guint32(DBusMessageIter *it, guint32 *value);
    void dbus_put_gint32(DBusMessageIter *it, gint32 *value);
    void dbus_put_guint64(DBusMessageIter *it, guint64 *value);
    void dbus_put_gint64(DBusMessageIter *it, gint64 *value);
    void dbus_put_bool(DBusMessageIter *it, bool *value);
    void dbus_put_double(DBusMessageIter *it, double *value);
    void dbus_put_string(DBusMessageIter *it, const std::string *value);

  protected:
    //! 
    DBusConnection *connection;  
  };
}

#endif // DBUSBINDING_HH
