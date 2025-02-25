// Copyright (C) 2015 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_DBUS_DBUSFACTORY_HH
#define WORKRAVE_DBUS_DBUSFACTORY_HH

#include <memory>

#include "dbus/IDBus.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusFactory
    {
    public:
      static IDBus::Ptr create();
    };
  } // namespace dbus
} // namespace workrave
#endif // WORKRAVE_DBUS_IDBUS_HH
