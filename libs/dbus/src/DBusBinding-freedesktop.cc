// DBusBinding.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include "DBus-freedesktop.hh"
#include "DBusBinding-freedesktop.hh"
#include "DBusException.hh"

using namespace workrave;


DBusBindingBase::DBusBindingBase(DBus *dbus)
  : dbus(dbus)
{
}


DBusBindingBase::~DBusBindingBase()
{
}


DBusMessage *
DBusBindingBase::call(const std::string &method, void *object, DBusMessage *message)
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
      throw DBusUsageException(std::string("No such member:") + method );
    }

  return ret;
}


void
DBusBaseTypes::get_uint8(DBusMessageIter *it, guint8 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_BYTE)
    {
      throw DBusTypeException("UInt8 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_uint16(DBusMessageIter *it, guint16 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT16)
    {
      throw DBusTypeException("UInt16 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_int16(DBusMessageIter *it, gint16 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT16)
    {
      throw DBusTypeException("Int16 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}

void
DBusBaseTypes::get_uint32(DBusMessageIter *it, guint32 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT32)
    {
      throw DBusTypeException("UInt32 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_int32(DBusMessageIter *it, gint32 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT32)
    {
      throw DBusTypeException("Int32 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_uint64(DBusMessageIter *it, guint64 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_UINT64)
    {
      throw DBusTypeException("UInt64 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_int64(DBusMessageIter *it, gint64 *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_INT64)
    {
      throw DBusTypeException("Int64 expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_bool(DBusMessageIter *it, bool *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_BOOLEAN)
    {
      throw DBusTypeException("Boolean expected");
    }

  gboolean v;
	dbus_message_iter_get_basic(it, &v);
  dbus_message_iter_next(it);

  *value = v;
}


void
DBusBaseTypes::get_double(DBusMessageIter *it, double *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_DOUBLE)
    {
      throw DBusTypeException("Double expected");
    }

	dbus_message_iter_get_basic(it, value);
  dbus_message_iter_next(it);
}


void
DBusBaseTypes::get_string(DBusMessageIter *it, std::string *value)
{
	int argtype = dbus_message_iter_get_arg_type(it);

  if (argtype != DBUS_TYPE_STRING)
    {
      throw DBusTypeException("String expected");
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
DBusBaseTypes::put_uint8(DBusMessageIter *it, const guint8 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_BYTE, value);
}

void
DBusBaseTypes::put_uint16(DBusMessageIter *it, const guint16 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_UINT16, value);
}


void
DBusBaseTypes::put_int16(DBusMessageIter *it, const gint16 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT16, value);
}

void
DBusBaseTypes::put_uint32(DBusMessageIter *it, const guint32 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_UINT32, value);
}


void
DBusBaseTypes::put_int32(DBusMessageIter *it, const gint32 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, value);
}


void
DBusBaseTypes::put_uint64(DBusMessageIter *it, const guint64 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_UINT64, value);
}


void
DBusBaseTypes::put_int64(DBusMessageIter *it, const gint64 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT64, value);
}


void
DBusBaseTypes::put_double(DBusMessageIter *it, const double *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_DOUBLE, value);
}


void
DBusBaseTypes::put_bool(DBusMessageIter *it, const bool *value)
{
  gboolean v = *value;
	dbus_message_iter_append_basic(it, DBUS_TYPE_BOOLEAN, &v);
}


void
DBusBaseTypes::put_string(DBusMessageIter *it, const std::string *value)
{
  const char *cstr = value->c_str();
	dbus_message_iter_append_basic(it, DBUS_TYPE_STRING, &cstr);
}


void
DBusBindingBase::send(DBusMessage *msg)
{
  dbus->send(msg);
}
