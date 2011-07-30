// DBusException.hh --- DBUS interface
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

#ifndef DBUSEXCEPTION_HH
#define DBUSEXCEPTION_HH

#include "Exception.hh"

namespace workrave
{

  class DBusException : public workrave::Exception
  {
  public:
    explicit DBusException(const std::string &id, const std::string &detail)
      : Exception(detail), dbus_id(id)
    {
    }

    explicit DBusException(const std::string &detail)
      : Exception(detail), dbus_id("org.workrave.Error")
    {
    }

    virtual ~DBusException() throw()
    {
    }

    std::string id() const
    {
      return dbus_id;
    }

  private:
    std::string dbus_id;
  };


  class DBusSystemException : public DBusException
  {
  public:
    explicit DBusSystemException(const std::string &id, const std::string &detail)
      : DBusException(id, detail)
    {
    }

    explicit DBusSystemException(const std::string &detail)
      : DBusException("org.workrave.SystemError", detail)
    {
    }

    virtual ~DBusSystemException() throw()
    {
    }
  };


  class DBusTypeException : public DBusException
  {
  public:
    explicit DBusTypeException(const std::string &id, const std::string &detail)
      : DBusException(id, detail)
    {
    }

    explicit DBusTypeException(const std::string &detail)
      : DBusException("org.workrave.TypeError", detail)
    {
    }

    virtual ~DBusTypeException() throw()
    {
    }
  };


  class DBusUsageException : public DBusException
  {
  public:
    explicit DBusUsageException(const std::string &id, const std::string &detail)
      : DBusException(id, detail)
    {
    }

    explicit DBusUsageException(const std::string &detail)
      : DBusException("org.workrave.UsageError", detail)
    {
    }

    virtual ~DBusUsageException() throw()
    {
    }
  };
}

#endif // DBUSEXCEPTION_HH
