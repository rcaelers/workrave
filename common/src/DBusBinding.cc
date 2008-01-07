// DBusBinding.c
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
// $Id: DBus.cc 1380 2007-12-04 21:45:36Z rcaelers $
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include "DBusBinding.hh"
#include "DBusException.hh"

using namespace workrave;

DBusBindingBase::DBusBindingBase()
  : connection(NULL)
{
}


DBusBindingBase::~DBusBindingBase()
{
}


void
DBusBindingBase::init(DBusConnection *connection)
{
  this->connection = connection;
}


DBusMessage *
DBusBindingBase::call(const std::string &method, void *object, DBusMessage *dbus_message)
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
      ret = call(count, object, dbus_message);
    }
  else
    {
      throw DBusUsageException("No such member");
    }

  return ret;
}


void
DBusBindingBase::dbus_get_int(DBusMessageIter *it, int *value)
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
DBusBindingBase::dbus_get_guint32(DBusMessageIter *it, guint32 *value)
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
DBusBindingBase::dbus_get_gint32(DBusMessageIter *it, gint32 *value)
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
DBusBindingBase::dbus_get_guint64(DBusMessageIter *it, guint64 *value)
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
DBusBindingBase::dbus_get_gint64(DBusMessageIter *it, gint64 *value)
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
DBusBindingBase::dbus_get_bool(DBusMessageIter *it, bool *value)
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
DBusBindingBase::dbus_get_double(DBusMessageIter *it, double *value)
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
DBusBindingBase::dbus_get_string(DBusMessageIter *it, std::string *value)
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
DBusBindingBase::dbus_put_int(DBusMessageIter *it, int *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, value);
}


void
DBusBindingBase::dbus_put_guint32(DBusMessageIter *it, guint32 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_UINT32, value);
}


void
DBusBindingBase::dbus_put_gint32(DBusMessageIter *it, gint32 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT32, value);
}


void
DBusBindingBase::dbus_put_guint64(DBusMessageIter *it, guint64 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_UINT64, value);
}


void
DBusBindingBase::dbus_put_gint64(DBusMessageIter *it, gint64 *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_INT64, value);
}


void
DBusBindingBase::dbus_put_double(DBusMessageIter *it, double *value)
{
	dbus_message_iter_append_basic(it, DBUS_TYPE_DOUBLE, value);
}


void
DBusBindingBase::dbus_put_bool(DBusMessageIter *it, bool *value)
{
  gboolean v = *value;
	dbus_message_iter_append_basic(it, DBUS_TYPE_BOOLEAN, &v);
}


void
DBusBindingBase::dbus_put_string(DBusMessageIter *it, const std::string *value)
{
  const char *cstr = value->c_str();
	dbus_message_iter_append_basic(it, DBUS_TYPE_STRING, &cstr);
}
