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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"

#include "DBus-freedesktop.hh"
#include "DBusBinding-freedesktop.hh"
#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::dbus;

DBusBindingFreeDesktop::DBusBindingFreeDesktop(IDBus::Ptr dbus)
  : dbus(dbus)
{
}

DBusBindingFreeDesktop::~DBusBindingFreeDesktop()
{
}

DBusMessage *
DBusBindingFreeDesktop::call(const std::string &method, void *object, DBusMessage *message)
{
  DBusMessage *ret = NULL;
  bool found = false;
  int count = 0;

  DBusIntrospect *table = get_method_introspect();
  while (!found && table[count].name != NULL)
    {
      if (method == table[count].name)
        {
          found = true;
        }
      else
        {
          count++;
        }
    }

  if (found)
    {
      ret = call(count, object, message);
    }
  else
    {
      throw DBusRemoteException(std::string("No such member:") + method);
    }

  return ret;
}

void
DBusBindingFreeDesktop::get_uint8(DBusMessageIter *it, uint8_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_BYTE)
    {
      throw DBusRemoteException("UInt8 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_uint16(DBusMessageIter *it, uint16_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT16)
    {
      throw DBusRemoteException("UInt16 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_int16(DBusMessageIter *it, int16_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT16)
    {
      throw DBusRemoteException("Int16 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_uint32(DBusMessageIter *it, uint32_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT32)
    {
      throw DBusRemoteException("UInt32 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_int32(DBusMessageIter *it, int32_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT32)
    {
      throw DBusRemoteException("Int32 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_uint64(DBusMessageIter *it, uint64_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT64)
    {
      throw DBusRemoteException("UInt64 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_int64(DBusMessageIter *it, int64_t *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT64)
    {
      throw DBusRemoteException("Int64 expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_bool(DBusMessageIter *it, bool *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_BOOLEAN)
    {
      throw DBusRemoteException("Boolean expected");
    }

  gboolean v;
  dbus_message_iter_get_basic(it, &v);
  dbus_message_iter_next(it);

  *value = v;
}

void
DBusBindingFreeDesktop::get_double(DBusMessageIter *it, double *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_DOUBLE)
    {
      throw DBusRemoteException("Double expected");
    }

  dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBindingFreeDesktop::get_string(DBusMessageIter *it, std::string *value)
{
  int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_STRING)
    {
      throw DBusRemoteException("String expected");
    }

  char *cstr = NULL;
  dbus_message_iter_get_basic(it, &cstr);
  dbus_message_iter_next(it);

  if (cstr != NULL)
    {
      *value = cstr;
    }
}

void
DBusBindingFreeDesktop::put_uint8(DBusMessageIter *it, const uint8_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_BYTE, value);
}

void
DBusBindingFreeDesktop::put_uint16(DBusMessageIter *it, const uint16_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_UINT16, value);
}

void
DBusBindingFreeDesktop::put_int16(DBusMessageIter *it, const int16_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_INT16, value);
}

void
DBusBindingFreeDesktop::put_uint32(DBusMessageIter *it, const uint32_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_UINT32, value);
}

void
DBusBindingFreeDesktop::put_int32(DBusMessageIter *it, const int32_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, value);
}

void
DBusBindingFreeDesktop::put_uint64(DBusMessageIter *it, const uint64_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_UINT64, value);
}

void
DBusBindingFreeDesktop::put_int64(DBusMessageIter *it, const int64_t *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_INT64, value);
}

void
DBusBindingFreeDesktop::put_double(DBusMessageIter *it, const double *value)
{
  dbus_message_iter_append_basic(it, DBUS_TYPE_DOUBLE, value);
}

void
DBusBindingFreeDesktop::put_bool(DBusMessageIter *it, const bool *value)
{
  gboolean v = *value;
  dbus_message_iter_append_basic(it, DBUS_TYPE_BOOLEAN, &v);
}

void
DBusBindingFreeDesktop::put_string(DBusMessageIter *it, const std::string *value)
{
  const char *cstr = value->c_str();
  dbus_message_iter_append_basic(it, DBUS_TYPE_STRING, &cstr);
}

void
DBusBindingFreeDesktop::send(DBusMessage *msg)
{
  dbus->send(msg);
}
