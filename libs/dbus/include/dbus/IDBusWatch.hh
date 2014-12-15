// IDBusWatch.hh --- DBUS bus watch interface
//
// Copyright (C) 2007, 2008, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_DBUS_IDBUSWATCH_HH
#define WORKRAVE_DBUS_IDBUSWATCH_HH

#include <string>

namespace workrave
{
  namespace dbus
  {
    class IDBusWatch
    {
    public:
      virtual ~IDBusWatch() {}
      virtual void bus_name_presence(const std::string &name, bool present) = 0;
    };
  }
}

#endif // WORKRAVE_DBUS_IDBUSWATCH_HH
