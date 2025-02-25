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

#ifndef WORKRAVE_DBUS_DBUSDUMMY_HH
#define WORKRAVE_DBUS_DBUSDUMMY_HH

#include <string>

#include <memory>

#include "dbus/IDBus.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusDummy : public IDBus
    {
    public:
      using Ptr = std::shared_ptr<DBusDummy>;

    public:
      DBusDummy() = default;
      ~DBusDummy() override = default;

      // IDBus
      void init() override;
      void register_service(const std::string &service, IDBusWatch *cb = nullptr) override;
      void register_object_path(const std::string &object_path) override;
      void connect(const std::string &object_path, const std::string &interface_name, void *object) override;
      void disconnect(const std::string &object_path, const std::string &interface_name) override;

      void register_binding(const std::string &interface_name, DBusBinding *binding) override;
      DBusBinding *find_binding(const std::string &interface_name) const override;

      bool is_available() const override;
      bool is_running(const std::string &name) const override;

      void watch(const std::string &name, IDBusWatch *cb) override;
      void unwatch(const std::string &name) override;
    };
  } // namespace dbus
} // namespace workrave
#endif // WORKRAVE_DBUS_DBUSDUMMY_HH
