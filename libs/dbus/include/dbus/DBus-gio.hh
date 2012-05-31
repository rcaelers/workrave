// DBus.hh --- DBUS interface
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

#ifndef DBUSGIO_HH
#define DBUSGIO_HH

#include <glib.h>
#include <gio/gio.h>

#include <string>
#include <map>
#include <list>

#include "IDBusWatch.hh"

namespace workrave
{
  class DBusBindingBase;

  class DBus
  {
  public:
    DBus();
    ~DBus();

    void init();
    void register_service(const std::string &service);
    void register_object_path(const std::string &object_path);
    void connect(const std::string &path, const std::string &interface_name, void *object);
    void disconnect(const std::string &path, const std::string &interface_name);

    void register_binding(const std::string &interface_name, DBusBindingBase *binding);
    DBusBindingBase *find_binding(const std::string &interface_name) const;

    bool is_available() const;
    bool is_running(const std::string &name) const;

    GDBusConnection *get_connection() const { return connection; }

    void watch(const std::string &name, IDBusWatch *cb);
    void unwatch(const std::string &name);
    
  private:
    typedef std::map<std::string, DBusBindingBase *> Bindings;
    typedef Bindings::iterator BindingIter;
    typedef Bindings::const_iterator BindingCIter;

    typedef std::map<std::string, guint> Services;
    typedef Services::iterator ServicesIter;
    typedef Services::const_iterator ServicesCIter;
    
    struct InterfaceData
    {
      InterfaceData() : introspection_data(NULL), registration_id(0), object(NULL) {}

      std::string object_path;
      std::string interface_name;
      GDBusNodeInfo *introspection_data;
      guint registration_id;
      void *object;
    };
    
    typedef std::map<std::string, InterfaceData> Interfaces;
    typedef Interfaces::iterator InterfaceIter;
    typedef Interfaces::const_iterator InterfaceCIter;

    struct ObjectData
    {
      ObjectData() : registered(false) {}
      
      Interfaces interfaces;
      bool registered;
    };
      
    typedef std::map<std::string, ObjectData> Objects;
    typedef Objects::iterator ObjectIter;
    typedef Objects::const_iterator ObjectCIter;

    struct WatchData
    {
      guint id;
      IDBusWatch *callback;
      bool seen;
    };
      
    typedef std::map<std::string, WatchData> Watched;
    typedef Watched::iterator WatchIter;
    typedef Watched::const_iterator WatchCIter;
    
    void *find_object(const std::string &path, const std::string &interface_name) const;
    void send() const;

    std::string get_introspect(const std::string &path, const std::string &interface_name);
    void update_object_registration(InterfaceData &data);

    static void on_bus_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data);
    static void on_bus_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data);

    void bus_name_presence(const std::string &name, bool present);
    
    static void on_method_call(GDBusConnection       *connection,
                               const gchar           *sender,
                               const gchar           *object_path,
                               const gchar           *interface_name,
                               const gchar           *method_name,
                               GVariant              *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer               user_data);

    static GVariant *
    on_get_property(GDBusConnection  *connection,
                    const gchar      *sender,
                    const gchar      *object_path,
                    const gchar      *interface_name,
                    const gchar      *property_name,
                    GError          **error,
                    gpointer          user_data);
    

    static gboolean
    on_set_property (GDBusConnection  *connection,
                     const gchar      *sender,
                     const gchar      *object_path,
                     const gchar      *interface_name,
                     const gchar      *property_name,
                     GVariant         *value,
                     GError          **error,
                     gpointer          user_data);
    
    static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
    static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data);
   
  private:
    //!
    Services services;
    
    //! Bindings for interfaces.
    Bindings bindings;

    //!
    Objects objects;

    //
    Watched watched;
    
    //!
    bool owner;

    GDBusConnection *connection;

    static const GDBusInterfaceVTable interface_vtable;
  };
}

#endif // DBUSGIO_HH
