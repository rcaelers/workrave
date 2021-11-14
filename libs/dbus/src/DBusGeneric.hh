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

#ifndef WORKRAVE_DBUS_DBUSGENERIC_HH
#define WORKRAVE_DBUS_DBUSGENERIC_HH

#include <string>
#include <map>
#include <list>

#include <memory>

#include "dbus/IDBus.hh"
#include "dbus/DBusBinding.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusGeneric : public IDBus
    {
    public:
      using Ptr = std::shared_ptr<DBusGeneric>;

    public:
      DBusGeneric() = default;
      ~DBusGeneric() override = default;

      void connect(const std::string &path, const std::string &interface_name, void *object) override;
      void disconnect(const std::string &path, const std::string &interface_name) override;

      void register_binding(const std::string &interface_name, DBusBinding *binding) override;
      DBusBinding *find_binding(const std::string &interface_name) const override;

    protected:
      using Bindings = std::map<std::string, DBusBinding *>;
      using BindingIter = Bindings::iterator;
      using BindingCIter = Bindings::const_iterator;

      using Interfaces = std::map<std::string, void *>;
      using InterfaceIter = Interfaces::iterator;
      using InterfaceCIter = Interfaces::const_iterator;

      using Objects = std::map<std::string, Interfaces>;
      using ObjectIter = Objects::iterator;
      using ObjectCIter = Objects::const_iterator;

      void *find_object(const std::string &path, const std::string &interface_name) const;

      friend class DBusBinding;

      //! Bindings for interfaces.
      Bindings bindings;

      //!
      Objects objects;
    };
  } // namespace dbus
} // namespace workrave
#endif // WORKRAVE_DBUS_DBUSGENERIC_HH
