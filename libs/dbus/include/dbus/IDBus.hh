// DBus.hh --- DBUS interface
//
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

#ifndef WORKRAVE_DBUS_IDBUS_HH
#define WORKRAVE_DBUS_IDBUS_HH

#include <boost/shared_ptr.hpp>

#include "dbus/DBusException.hh"

namespace workrave
{
  namespace dbus
  {
    class IDBusWatch;
    class DBusBinding;
    
    class IDBus
    {
    public:
      typedef boost::shared_ptr<IDBus> Ptr;

    public:
      static Ptr create();

      virtual ~IDBus() {}

      virtual void init() = 0;
      virtual void register_service(const std::string &service) = 0;
      virtual void register_object_path(const std::string &object_path) = 0;
      virtual void connect(const std::string &object_path, const std::string &interface_name, void *object) = 0;
      virtual void disconnect(const std::string &object_path, const std::string &interface_name) = 0;

      virtual void register_binding(const std::string &interface_name, DBusBinding *binding) = 0;
      virtual DBusBinding *find_binding(const std::string &interface_name) const = 0;

      virtual bool is_available() const = 0;
      virtual bool is_running(const std::string &name) const = 0;

      virtual void watch(const std::string &name, IDBusWatch *cb) = 0;
      virtual void unwatch(const std::string &name) = 0;
    };
  }
}
#endif // WORKRAVE_DBUS_IDBUS_HH
