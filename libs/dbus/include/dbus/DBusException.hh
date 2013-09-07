// DBusException.hh --- DBUS interface
//
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

#ifndef WORKRAVE_DBUS_DBUSEXCEPTION_HH
#define WORKRAVE_DBUS_DBUSEXCEPTION_HH

#include "utils/Exception.hh"

namespace workrave
{
  namespace dbus
  {
    static const char *DBUS_ERROR_FAILED =                       "org.freedesktop.DBus.Error.Failed";
    static const char *DBUS_ERROR_NOT_SUPPORTED =                "org.freedesktop.DBus.Error.NotSupported";
    static const char *DBUS_ERROR_INVALID_ARGS =                 "org.freedesktop.DBus.Error.InvalidArgs";
    static const char *DBUS_ERROR_UNKNOWN_METHOD =               "org.freedesktop.DBus.Error.UnknownMethod";

    class DBusException : public workrave::utils::Exception
    {
    public:
      explicit DBusException(const std::string &detail)
        : Exception(detail)
      {
      }

      virtual ~DBusException() throw()
      {
      }
    };

    class DBusRemoteException : public DBusException
    {
    public:
      explicit DBusRemoteException(const std::string &id, const std::string &detail)
        : DBusException(detail), dbus_id(id)
      {
      }

      explicit DBusRemoteException(const std::string &detail)
        : DBusException(detail), dbus_id("org.freedesktop.DBus.Error.Failed")
      {
      }

      virtual ~DBusRemoteException() throw()
      {
      }

      std::string id() const
      {
        return dbus_id;
      }

    private:
      std::string dbus_id;
    };
  }
}

#endif // WORKRAVE_DBUS_DBUSEXCEPTION_HH
