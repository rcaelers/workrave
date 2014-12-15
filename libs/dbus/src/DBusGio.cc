// DBus-gio.c
//
// Copyright (C) 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DBusGio.hh"
#include "debug.hh"

#include <string>
#include <list>
#include <map>

#include <gio/gio.h>

#include "dbus/DBusException.hh"
#include "dbus/DBusBindingGio.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;
using namespace workrave::utils;

const GDBusInterfaceVTable DBusGio::interface_vtable =
{
  &DBusGio::on_method_call,
  &DBusGio::on_get_property,
  &DBusGio::on_set_property,
  { NULL, }
};

DBusGio::Ptr
DBusGio::create()
{
  return Ptr(new DBusGio());
}


//! Construct a new D-BUS bridge
DBusGio::DBusGio()
  : connection(NULL)
{
}


//! Destruct the D-BUS bridge
DBusGio::~DBusGio()
{
  for (ServicesCIter i = services.begin(); i != services.end(); i++)
    {
      g_bus_unown_name(i->second);
    }

  for (auto &obj : objects)
    {
      for (InterfaceIter interface_it = obj.second.interfaces.begin();
           interface_it != obj.second.interfaces.end();
           interface_it++)
        {
          if (interface_it->second.registration_id != 0)
            {
              g_dbus_connection_unregister_object(connection, interface_it->second.registration_id);
            }

          if (interface_it->second.introspection_data != NULL)
            {
              g_dbus_node_info_unref(interface_it->second.introspection_data);
            }
        }
    }
}


//! Initialize D-BUS bridge
void
DBusGio::init()
{
}


//! Registers the specified service
void
DBusGio::register_service(const std::string &service_name)
{
  guint owner_id;

  owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                            service_name.c_str(),
                            G_BUS_NAME_OWNER_FLAGS_NONE,
                            &DBusGio::on_bus_acquired,
                            NULL, //&DBusGio::on_name_acquired,
                            NULL, //&DBusGio::on_name_lost,
                            this,
                            NULL);

  services[service_name] = owner_id;
}


//! Registers the specified object path
void
DBusGio::register_object_path(const string &object_path)
{
  objects[object_path].registered = true;
}


void
DBusGio::update_object_registration(InterfaceData &data)
{
  TRACE_ENTER_MSG("DBusGio::update_object_registration", data.object_path);
  if (connection == NULL)
    {
      TRACE_RETURN("No Connection");
      return;
    }
  
  if (data.registration_id != 0)
    {
      g_dbus_connection_unregister_object(connection, data.registration_id);
    }
      
  string introspection_xml = get_introspect(data.object_path, data.interface_name);
  TRACE_MSG("Intro: %s" << introspection_xml);

  GError *error = NULL;
  data.introspection_data = g_dbus_node_info_new_for_xml(introspection_xml.c_str(), &error);

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }
  
  data.registration_id = g_dbus_connection_register_object(connection,
                                                           data.object_path.c_str(),
                                                           data.introspection_data->interfaces[0],
                                                           &interface_vtable,
                                                           this, NULL, NULL);

  TRACE_EXIT();
}


//! Connect a D-DBUS object/interface to a C object
void
DBusGio::connect(const std::string &object_path, const std::string &interface_name, void *object)
{
  DBusBindingGio *binding = dynamic_cast<DBusBindingGio *>(find_binding(interface_name));
  if (binding == NULL)
    {
      throw DBusException("No such interface");
    }

  ObjectIter oit = objects.find(object_path);
  if (oit == objects.end())
    {
      objects[object_path].registered = false;
    }

  ObjectData &object_data = objects[object_path];

  InterfaceIter iit =  object_data.interfaces.find(interface_name);
  if (iit != object_data.interfaces.end())
    {
      throw DBusException("Interface already registered");
    }

  InterfaceData &interface_data = object_data.interfaces[interface_name];
  interface_data.object_path = object_path;
  interface_data.interface_name = interface_name;
  interface_data.object = object;

  if (object_data.registered)
    {
      update_object_registration(interface_data);
    }
}


//! Disconnect a D-DBUS object/interface to a C object
void
DBusGio::disconnect(const std::string &object_path, const std::string &interface_name)
{
  ObjectIter it = objects.find(object_path);
  if (it != objects.end())
    {
      Interfaces &interfaces = it->second.interfaces;

      if (interfaces[interface_name].registration_id != 0)
        {
          g_dbus_connection_unregister_object(connection, interfaces[interface_name].registration_id);
        }

      if (interfaces[interface_name].introspection_data != NULL)
        {
          g_dbus_node_info_unref(interfaces[interface_name].introspection_data);
        }
      
      interfaces.erase(interface_name);
    }
}


//! Register an interface binding
void
DBusGio::register_binding(const std::string &name, DBusBinding *interface)
{
  bindings[name] = interface;
}


//! Find an interface binding
DBusBinding *
DBusGio::find_binding(const std::string &interface_name) const
{
  DBusBinding *ret = NULL;

  BindingCIter it = bindings.find(interface_name);
  if (it != bindings.end())
    {
      ret = it->second;
    }

  return ret;
}


void *
DBusGio::find_object(const std::string &path, const std::string &interface_name) const
{
  void *object = NULL;

  ObjectCIter object_it = objects.find(path);
  if (object_it != objects.end())
    {
      InterfaceCIter interface_it = object_it->second.interfaces.find(interface_name);

      if (interface_it != object_it->second.interfaces.end())
        {
          object = interface_it->second.object;
        }
    }

  return object;
}


bool
DBusGio::is_running(const std::string &name) const
{
  TRACE_ENTER("DBusGio::is_runninf");
	GError *error = NULL;
	gboolean running = FALSE;

  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    NULL,
                                                    "org.freedesktop.DBus",
                                                    "/org/freedesktop/DBus",
                                                    "org.freedesktop.DBus",
                                                    NULL,
                                                    &error);

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }

  if (error == NULL && proxy != NULL)
    {
      GVariant *result = g_dbus_proxy_call_sync(proxy,
                                                "NameHasOwner",
                                                g_variant_new("(s)", name.c_str()),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);

      if (error != NULL)
        {
          TRACE_MSG("Error: " << error->message);
          g_error_free(error);
        }
      else
        {
          GVariant *first = g_variant_get_child_value(result, 0);
          running = g_variant_get_boolean(first);
          g_variant_unref(first);
          g_variant_unref(result);
        }
    }

  TRACE_RETURN(running);
	return running;
}


bool
DBusGio::is_available() const
{
  return true;
}

void
DBusGio::on_bus_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data)
{
  (void) connection;
  (void) name_owner;
  DBusGio *dbus = (DBusGio *)user_data;
  dbus->watched[name].seen = true;
  dbus->bus_name_presence(name, true);
}

void
DBusGio::on_bus_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void) connection;
  DBusGio *dbus = (DBusGio *)user_data;
  if (dbus->watched[name].seen)
    {
      dbus->bus_name_presence(name, false);
    }
}

void
DBusGio::bus_name_presence(const std::string &name, bool present)
{
  if (watched.find(name) != watched.end())
    {
      watched[name].callback->bus_name_presence(name, present);
    }
}

void
DBusGio::watch(const std::string &name, IDBusWatch *cb)
{
  guint id = g_bus_watch_name_on_connection(connection,
                                            name.c_str(),
                                            G_BUS_NAME_WATCHER_FLAGS_NONE,
                                            on_bus_name_appeared,
                                            on_bus_name_vanished,
                                            this,
                                            NULL);
  watched[name].callback = cb;
  watched[name].id = id;
  watched[name].seen = false;
}

void
DBusGio::unwatch(const std::string &name)
{
  guint id = watched[name].id;
  g_bus_unwatch_name(id);
  watched.erase(name);  
}


string
DBusGio::get_introspect(const string &object_path, const string &interface_name)
{
  TRACE_ENTER_MSG("DBusGio::get_introspect", object_path);
	string str;
 
  str += "<!DOCTYPE node PUBLIC '-//freedesktop//DTD D-BUS Object Introspection 1.0//EN' 'http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd'>\n";
  str += "<node name='" + object_path + "'>\n";
 
  ObjectCIter object_it = objects.find(object_path);
  if (object_it != objects.end())
    {
      InterfaceCIter interface_it = object_it->second.interfaces.find(interface_name);
      if (interface_it != object_it->second.interfaces.end())
        {
          DBusBindingGio *binding = dynamic_cast<DBusBindingGio*>(find_binding(interface_it->first));
          if (binding == NULL)
            {
              throw DBusRemoteException()
                << message_info("Unknown interface")
                << error_code_info(DBUS_ERROR_FAILED)
                << interface_info(interface_name);
            }

          const char *interface_introspect = binding->get_interface_introspect();
          str += string(interface_introspect);
        }
    }

  str += "</node>\n";
  TRACE_RETURN(str);
  return str;
}


void
DBusGio::on_method_call(GDBusConnection       *connection,
                     const gchar           *sender,
                     const gchar           *object_path,
                     const gchar           *interface_name,
                     const gchar           *method_name,
                     GVariant              *parameters,
                     GDBusMethodInvocation *invocation,
                     gpointer               user_data)
{
  (void) connection;
  (void) sender;
  
  try
    {
      DBusGio *self = (DBusGio *) user_data;

      void *object = self->find_object(object_path, interface_name);
      if (object == NULL)
        {
          throw DBusRemoteException()
            << message_info("No such object")
            << error_code_info(DBUS_ERROR_FAILED)
            << object_info(object_path)
            << interface_info(interface_name);
        }

      DBusBindingGio *binding = dynamic_cast<DBusBindingGio*>(self->find_binding(interface_name));
      if (binding == NULL)
        {
          throw DBusRemoteException()
            << message_info("No such interface")
            << error_code_info(DBUS_ERROR_FAILED)
            << object_info(object_path)
            << interface_info(interface_name);
        }

      binding->call(method_name, object, invocation, sender, parameters);
    }
  catch (DBusRemoteException &e)
    {
      std::cout << "error : " << e.diag() << std::endl;

      g_dbus_method_invocation_return_error (invocation,
                                             G_IO_ERROR,
                                             G_IO_ERROR_FAILED_HANDLED,
                                             "%s", e.diag().c_str());
    }
}


GVariant *
DBusGio::on_get_property(GDBusConnection  *connection,
                      const gchar      *sender,
                      const gchar      *object_path,
                      const gchar      *interface_name,
                      const gchar      *property_name,
                      GError          **error,
                      gpointer          user_data)
{
  (void) connection;
  (void) sender;
  (void) object_path;
  (void) interface_name;
  (void) property_name;
  (void) error;
  (void) user_data;

  return NULL;
}


gboolean
DBusGio::on_set_property(GDBusConnection  *connection,
                      const gchar      *sender,
                      const gchar      *object_path,
                      const gchar      *interface_name,
                      const gchar      *property_name,
                      GVariant         *value,
                      GError          **error,
                      gpointer          user_data)
{
  (void) connection;
  (void) sender;
  (void) object_path;
  (void) interface_name;
  (void) property_name;
  (void) value;
  (void) error;
  (void) user_data;

  return FALSE;
}


void
DBusGio::on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void) name;
  TRACE_ENTER_MSG("DBusGio::on_bus_acquired", name);

  DBusGio *self = (DBusGio *) user_data;
  self->connection = connection;
  
  for (ObjectIter object_it = self->objects.begin();object_it != self->objects.end(); object_it++)
    {
      for (auto &iface : object_it->second.interfaces)
        {
          self->update_object_registration(iface.second);
        }
    }
  TRACE_EXIT();
}


void
DBusGio::on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void) connection;
  (void) name;
  (void) user_data;
  TRACE_ENTER_MSG("DBusGio::on_name_acquired", name);
  TRACE_EXIT();
}


void
DBusGio::on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void) connection;
  (void) name;
  (void) user_data;
  TRACE_ENTER_MSG("DBusGio::on_name_lost", name);
  TRACE_EXIT();
}
