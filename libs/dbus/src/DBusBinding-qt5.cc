// DBusBinding-gio.c
//
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include "dbus/DBus-qt5.hh"
#include "dbus/DBusBinding-qt5.hh"
#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::dbus;


DBusBindingBase::DBusBindingBase(DBus *dbus)
  :dbus(dbus)
{
}


DBusBindingBase::~DBusBindingBase()
{
}

void
DBusBaseTypes::get_uint8(const QVariant &variant, uint8_t *value)
{
  *value = variant.value<uint8_t>();
}

void
DBusBaseTypes::get_uint16(const QVariant &variant, uint16_t *value)
{
  *value = variant.value<uint16_t>();
}

void
DBusBaseTypes::get_int16(const QVariant &variant, int16_t *value)
{
  *value = variant.value<int16_t>();
}

void
DBusBaseTypes::get_uint32(const QVariant &variant, uint32_t *value)
{
  *value = variant.value<uint32_t>();
}

void
DBusBaseTypes::get_int32(const QVariant &variant, int32_t *value)
{
  *value = variant.value<int32_t>();
}

void
DBusBaseTypes::get_uint64(const QVariant &variant, uint64_t *value)
{
  *value = variant.value<uint64_t>();
}

void
DBusBaseTypes::get_int64(const QVariant &variant, int64_t *value)
{
  *value = variant.value<int64_t>();
}

void
DBusBaseTypes::get_bool(const QVariant &variant, bool *value)
{
  *value = variant.value<bool>();
}

void
DBusBaseTypes::get_double(const QVariant &variant, double *value)
{
  *value = variant.value<double>();
}

void
DBusBaseTypes::get_string(const QVariant &variant, std::string *value)
{
  *value = variant.value<QString>().toStdString();
}

QVariant
DBusBaseTypes::put_uint8(const uint8_t *value)
{
	return QVariant(*value);
}

QVariant
DBusBaseTypes::put_uint16(const uint16_t *value)
{
	return QVariant(*value);
}

QVariant
DBusBaseTypes::put_int16(const int16_t *value)
{
	return QVariant(*value);
}

QVariant
DBusBaseTypes::put_uint32(const uint32_t *value)
{
	return QVariant(*value);
}


QVariant
DBusBaseTypes::put_int32(const int32_t *value)
{
	return QVariant(*value);
}


QVariant
DBusBaseTypes::put_uint64(const uint64_t *value)
{
	return QVariant((qulonglong)*value);
}

QVariant
DBusBaseTypes::put_int64(const int64_t *value)
{
	return QVariant((qlonglong)*value);
}


QVariant
DBusBaseTypes::put_double(const double_t *value)
{
	return QVariant(*value);
}

QVariant
DBusBaseTypes::put_bool(const bool *value)
{
	return QVariant(*value);
}

QVariant
DBusBaseTypes::put_string(const std::string *value)
{
	return QVariant(QString::fromStdString(*value));
}
