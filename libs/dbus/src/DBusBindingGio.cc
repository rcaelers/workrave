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
#include <gio/gio.h>

#include "DBusGio.hh"
#include "dbus/DBusBindingGio.hh"
#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::dbus;

DBusBindingGio::DBusBindingGio(std::shared_ptr<IDBus> dbus)
  : dbus(dbus)
{
}

void
DBusMarshallGio::get_int(GVariant *v, int *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT32))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("int");
    }

  *value = g_variant_get_int32(v);
}

void
DBusMarshallGio::get_uint8(GVariant *v, uint8_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_BYTE))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("uint8");
    }

  *value = g_variant_get_byte(v);
}

void
DBusMarshallGio::get_uint16(GVariant *v, uint16_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT16))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("uint16");
    }

  *value = g_variant_get_uint16(v);
}

void
DBusMarshallGio::get_int16(GVariant *v, int16_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT16))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("int16");
    }

  *value = g_variant_get_int16(v);
}

void
DBusMarshallGio::get_uint32(GVariant *v, uint32_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT32))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("uint32");
    }

  *value = g_variant_get_uint32(v);
}

void
DBusMarshallGio::get_int32(GVariant *v, int32_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT32))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("int32");
    }

  *value = g_variant_get_int32(v);
}

void
DBusMarshallGio::get_uint64(GVariant *v, uint64_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_UINT64))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("uint64");
    }

  *value = g_variant_get_uint64(v);
}

void
DBusMarshallGio::get_int64(GVariant *v, int64_t *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_INT64))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("int64");
    }

  *value = g_variant_get_int64(v);
}

void
DBusMarshallGio::get_bool(GVariant *v, bool *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_BOOLEAN))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("bool");
    }

  *value = g_variant_get_boolean(v);
}

void
DBusMarshallGio::get_double(GVariant *v, double *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_DOUBLE))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("double");
    }

  *value = g_variant_get_double(v);
}

void
DBusMarshallGio::get_string(GVariant *v, std::string *value)
{
  const GVariantType *argtype = g_variant_get_type(v);

  if (!g_variant_type_equal(argtype, G_VARIANT_TYPE_STRING))
    {
      throw DBusRemoteException() << message_info("Type error") << error_code_info(DBUS_ERROR_INVALID_ARGS)
                                  << expected_type_info("string");
    }

  const char *cstr = g_variant_get_string(v, nullptr);
  if (cstr != nullptr)
    {
      *value = cstr;
    }
}

GVariant *
DBusMarshallGio::put_int(const int *value)
{
  return g_variant_new_int32(*value);
}

GVariant *
DBusMarshallGio::put_uint8(const uint8_t *value)
{
  return g_variant_new_byte(*value);
}

GVariant *
DBusMarshallGio::put_uint16(const uint16_t *value)
{
  return g_variant_new_uint16(*value);
}

GVariant *
DBusMarshallGio::put_int16(const int16_t *value)
{
  return g_variant_new_int16(*value);
}

GVariant *
DBusMarshallGio::put_uint32(const uint32_t *value)
{
  return g_variant_new_uint32(*value);
}

GVariant *
DBusMarshallGio::put_int32(const int32_t *value)
{
  return g_variant_new_int32(*value);
}

GVariant *
DBusMarshallGio::put_uint64(const uint64_t *value)
{
  return g_variant_new_uint64(*value);
}

GVariant *
DBusMarshallGio::put_int64(const int64_t *value)
{
  return g_variant_new_int64(*value);
}

GVariant *
DBusMarshallGio::put_double(const double *value)
{
  return g_variant_new_double(*value);
}

GVariant *
DBusMarshallGio::put_bool(const bool *value)
{
  gboolean v = *value;
  return g_variant_new_boolean(v);
}

GVariant *
DBusMarshallGio::put_string(const std::string *value)
{
  const char *cstr = value->c_str();
  return g_variant_new_string(cstr);
}
