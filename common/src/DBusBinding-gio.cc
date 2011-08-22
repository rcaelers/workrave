// DBusBinding-gio.c
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
#include <gio/gio.h>

#include "DBus-gio.hh"
#include "DBusBinding-gio.hh"
#include "DBusException.hh"

using namespace workrave;


DBusBindingBase::DBusBindingBase(DBus *dbus)
  : dbus(dbus)
{
}


DBusBindingBase::~DBusBindingBase()
{
}

void
DBusBaseTypes::get_uint8(GVariant *v, guint8 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_BYTE))
    {
      throw DBusTypeException("UInt8 expected");
    }

	*value = g_variant_get_byte(v);
}


void
DBusBaseTypes::get_uint16(GVariant *v, guint16 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT16))
    {
      throw DBusTypeException("UInt16 expected");
    }

	*value = g_variant_get_uint16(v);
}


void
DBusBaseTypes::get_int16(GVariant *v, gint16 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT16))
    {
      throw DBusTypeException("Int16 expected");
    }

	*value = g_variant_get_int16(v);
}

void
DBusBaseTypes::get_uint32(GVariant *v, guint32 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT32))
    {
      throw DBusTypeException("UInt32 expected");
    }

	*value = g_variant_get_uint32(v);
}


void
DBusBaseTypes::get_int32(GVariant *v, gint32 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);
  
  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT32))
    {
      throw DBusTypeException("Int32 expected");
    }

	*value = g_variant_get_int32(v);
}


void
DBusBaseTypes::get_uint64(GVariant *v, guint64 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT64))
    {
      throw DBusTypeException("UInt64 expected");
    }

	*value = g_variant_get_uint64(v);
}


void
DBusBaseTypes::get_int64(GVariant *v, gint64 *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT64))
    {
      throw DBusTypeException("Int64 expected");
    }

	*value = g_variant_get_int64(v);
}


void
DBusBaseTypes::get_bool(GVariant *v, bool *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_BOOLEAN))
    {
      throw DBusTypeException("Boolean expected");
    }

	*value = g_variant_get_boolean(v);
}


void
DBusBaseTypes::get_double(GVariant *v, double *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_DOUBLE))
    {
      throw DBusTypeException("Double expected");
    }

	*value = g_variant_get_double(v);
}


void
DBusBaseTypes::get_string(GVariant *v, std::string *value)
{
	const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_STRING))
    {
      throw DBusTypeException("String expected");
    }

  const char *cstr = g_variant_get_string(v, NULL);
  if (cstr != NULL)
    {
      *value = cstr;
    }
}


GVariant *
DBusBaseTypes::put_uint8(const guint8 *value)
{
	return g_variant_new_byte(*value);
}

GVariant *
DBusBaseTypes::put_uint16(const guint16 *value)
{
	return g_variant_new_uint16(*value);
}


GVariant *
DBusBaseTypes::put_int16(const gint16 *value)
{
	return g_variant_new_int16(*value);
}

GVariant *
DBusBaseTypes::put_uint32(const guint32 *value)
{
	return g_variant_new_uint32(*value);
}


GVariant *
DBusBaseTypes::put_int32(const gint32 *value)
{
	return g_variant_new_int32(*value);
}


GVariant *
DBusBaseTypes::put_uint64(const guint64 *value)
{
	return g_variant_new_uint64(*value);
}


GVariant *
DBusBaseTypes::put_int64(const gint64 *value)
{
	return g_variant_new_int64(*value);
}


GVariant *
DBusBaseTypes::put_double(const double *value)
{
	return g_variant_new_double(*value);
}


GVariant *
DBusBaseTypes::put_bool(const bool *value)
{
  gboolean v = *value;
	return g_variant_new_boolean(v);
}


GVariant *
DBusBaseTypes::put_string(const std::string *value)
{
  const char *cstr = value->c_str();
	return g_variant_new_string(cstr);
}
