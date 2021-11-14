// Copyright (C) 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "dbus/DBusException.hh"

namespace workrave
{
  namespace dbus
  {
    const char *DBUS_ERROR_FAILED = "org.freedesktop.DBus.Error.Failed";
    const char *DBUS_ERROR_NOT_SUPPORTED = "org.freedesktop.DBus.Error.NotSupported";
    const char *DBUS_ERROR_INVALID_ARGS = "org.freedesktop.DBus.Error.InvalidArgs";
    const char *DBUS_ERROR_UNKNOWN_METHOD = "org.freedesktop.DBus.Error.UnknownMethod";
  } // namespace dbus
} // namespace workrave
