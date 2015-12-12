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
      typedef std::shared_ptr<DBusGeneric> Ptr;

    public:
      DBusGeneric();
      ~DBusGeneric() override;

      void connect(const std::string &path, const std::string &interface_name, void *object) override;
      void disconnect(const std::string &path, const std::string &interface_name) override;

      void register_binding(const std::string &interface_name, DBusBinding *binding) override;
      DBusBinding *find_binding(const std::string &interface_name) const override;

    protected:
      typedef std::map<std::string, DBusBinding *> Bindings;
      typedef Bindings::iterator BindingIter;
      typedef Bindings::const_iterator BindingCIter;

      typedef std::map<std::string, void *> Interfaces;
      typedef Interfaces::iterator InterfaceIter;
      typedef Interfaces::const_iterator InterfaceCIter;

      typedef std::map<std::string, Interfaces> Objects;
      typedef Objects::iterator ObjectIter;
      typedef Objects::const_iterator ObjectCIter;

      void *find_object(const std::string &path, const std::string &interface_name) const;

      friend class DBusBinding;

      //! Bindings for interfaces.
      Bindings bindings;

      //!
      Objects objects;
    };
  }
}
#endif // WORKRAVE_DBUS_DBUSGENERIC_HH
