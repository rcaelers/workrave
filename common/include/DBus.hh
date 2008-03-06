// DBus.hh --- DBUS interface
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: DBus.hh 1404 2008-01-07 20:48:30Z rcaelers $
//

#ifndef DBUS_HH
#define DBUS_HH

#include <dbus/dbus.h>
#include <glib.h>

#include <string>
#include <map>
#include <list>

namespace workrave
{
  class DBusBindingBase;
  
  class DBus
  {
  public:
    DBus();
    ~DBus();

    typedef DBusMessage *DBusSignal;
    
    void init();
    void register_service(const std::string &service);
    void register_object_path(const std::string &object_path);
    void connect(const std::string &path, const std::string &interface_name, void *object);
    void disconnect(const std::string &path, const std::string &interface_name);

    void register_binding(const std::string &interface_name, DBusBindingBase *binding);
    DBusBindingBase *find_binding(const std::string &interface_name) const;

    bool is_available() const;
    bool is_owner() const;

    DBusConnection *conn() { return connection; }
        
  private:
    typedef std::map<std::string, DBusBindingBase *> Bindings;
    typedef Bindings::iterator BindingIter;
    typedef Bindings::const_iterator BindingCIter;

    typedef std::map<std::string, void *> Interfaces;
    typedef Interfaces::iterator InterfaceIter;
    typedef Interfaces::const_iterator InterfaceCIter;

    typedef std::map<std::string, Interfaces> Objects;
    typedef Objects::iterator ObjectIter;
    typedef Objects::const_iterator ObjectCIter;

    DBusHandlerResult dispatch_static(DBusConnection *connection,
                                      DBusMessage *message);
  
    static DBusHandlerResult dispatch_static(DBusConnection *connection,
                                             DBusMessage *message,
                                             void *user_data);
  
    DBusHandlerResult dispatch(DBusConnection *connection, DBusMessage *message);
    DBusHandlerResult handle_introspect(DBusConnection *connection, DBusMessage *message);
    DBusHandlerResult handle_method(DBusConnection *connection, DBusMessage *message);

    void *find_cobject(const std::string &path, const std::string &interface_name) const;
    void send(DBusMessage *msg) const;

    friend class DBusBindingBase;


  private:
    //! Connection to the DBus.
    DBusConnection *connection;

    //! Bindings for interfaces.
    Bindings bindings;

    //!
    Objects objects;

    //!
    bool owner;
  };
}

#endif // DBUS_HH
