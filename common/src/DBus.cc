// DBus.c
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
// Based on code from pidgin.
//
// $Id: DBus.cc 1404 2008-01-07 20:48:30Z rcaelers $
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <string>
#include <list>
#include <map>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus-glib-lowlevel.h>

#include "DBus.hh"
#include "DBusBinding.hh"
#include "DBusException.hh"

using namespace std;
using namespace workrave;

//! Construct a new D-BUS bridge
DBus::DBus()
  : connection(NULL), owner(false)
{
}


//! Destruct the D-BUS bridge
DBus::~DBus()
{
  if (connection != NULL)
    {
      dbus_connection_unref(connection);
    }
}
  

//! Initialize D-BUS bridge
void
DBus::init()
{
	DBusError error;
  
	dbus_error_init(&error);
  
	connection = dbus_bus_get(DBUS_BUS_STARTER, &error);
  if (dbus_error_is_set(&error))
    {
      connection = NULL;
      dbus_error_free(&error);
      throw DBusSystemException("Unable to obtain session bus");
    }

	dbus_connection_set_exit_on_disconnect(connection, FALSE);
	dbus_connection_setup_with_g_main(connection, NULL);
}


//! Registers the specified service
void
DBus::register_service(const std::string &service)
{
	DBusError error;
  int result = 0;
  
	dbus_error_init(&error);
  
	result = dbus_bus_request_name(connection,
                                 service.c_str(),
                                 DBUS_NAME_FLAG_ALLOW_REPLACEMENT | DBUS_NAME_FLAG_DO_NOT_QUEUE,
                                 &error);

	if (dbus_error_is_set(&error))
    {
      dbus_connection_unref(connection);
      connection = NULL;

      dbus_error_free(&error);
      throw DBusSystemException("Unable to request service");
    }

	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
      owner = false;
    }

  owner = true;
}


//! Registers the specified object path
void
DBus::register_object_path(const string &object_path)
{
	static DBusObjectPathVTable vtable = { NULL,
                                         &DBus::dispatch_static,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL };

	if (!dbus_connection_register_object_path(connection,
                                            object_path.c_str(),
                                            &vtable,
                                            this))
	{
    throw DBusSystemException("Unable to register object");
	}
}


//! Connect a D-DBUS object/interface to a C object
void
DBus::connect(const std::string &object_path, const std::string &interface_name, void *cobject)
{
  DBusBindingBase *binding = find_binding(interface_name);
  if (binding == NULL)
    {
      throw DBusSystemException("No such interface");
    }

  ObjectIter it = objects.find(object_path);
  if (it != objects.end())
    {
      Interfaces &interfaces = it->second;

      interfaces[interface_name] = cobject;
    }
  else
    {
      Interfaces interfaces;
      interfaces[interface_name] = cobject;
      objects[object_path] = interfaces;
    }
}


//! Register an interface binding
void
DBus::register_binding(const std::string &name, DBusBindingBase *interface)
{
  bindings[name] = interface;
  interface->init(connection);
}


//! Find an interface binding
DBusBindingBase *
DBus::find_binding(const std::string &interface_name) const
{
  DBusBindingBase *ret = NULL;
  
  BindingCIter it = bindings.find(interface_name);
  if (it != bindings.end())
    {
      ret = it->second;
    }
  
  return ret;
}

void *
DBus::find_cobject(const std::string &path, const std::string &interface_name) const
{
  void *cobject = NULL;
  
  ObjectCIter object_it = objects.find(path);
  if (object_it != objects.end())
    {
      InterfaceCIter interface_it = object_it->second.find(interface_name);

      if (interface_it != object_it->second.end())
        {
          cobject = interface_it->second;
        }
    }
  
  
  return cobject;
}


bool
DBus::is_owner() const
{
  return owner;
}


bool
DBus::is_available() const
{
  return connection != NULL;
}


DBusHandlerResult
DBus::dispatch(DBusConnection *connection, DBusMessage *message)
{
  try
    {
      if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL &&
          dbus_message_has_interface(message, DBUS_INTERFACE_INTROSPECTABLE) &&
          dbus_message_has_member(message, "Introspect"))
        {
          return handle_introspect(connection, message);
        }
      else if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL)
        {
          return handle_method(connection, message);
        }
    }
  catch (DBusException &e)
    {
			DBusMessage *reply;
      reply = dbus_message_new_error(message,	e.id().c_str(), e.details().c_str());
			if (reply != NULL)
			{
				dbus_connection_send(connection, reply, NULL);
				dbus_message_unref(reply);
			}
    }
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


DBusHandlerResult
DBus::dispatch_static(DBusConnection *connection,
                      DBusMessage *message,
                      void *user_data)
{
  DBus *dbus = (DBus *) user_data;
  return dbus->dispatch(connection, message);
}


static const char *
dbus_gettext(const char **ptr)
{
	const char *text = *ptr;
	*ptr += strlen(text) + 1;
	return text;
}


DBusHandlerResult
DBus::handle_introspect(DBusConnection *connection, DBusMessage *message)
{
	string str;

  string path = dbus_message_get_path(message);

  ObjectCIter object_it = objects.find(path);
  if (object_it != objects.end())
    {
      str = "<!DOCTYPE node PUBLIC '-//freedesktop//DTD D-BUS Object Introspection 1.0//EN' 'http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd'>\n";
      str += "<node name='" + path + "'>\n";

      str += "<interface name=\"org.freedesktop.DBus.Introspectable\">\n";
      str += "<method name=\"Introspect\">\n";
      str += "<arg name=\"data\" direction=\"out\" type=\"s\"/>\n";
      str += "</method>\n";
      str += "</interface>\n";
      
      for (InterfaceCIter interface_it = object_it->second.begin();
           interface_it != object_it->second.end();
           interface_it++)
        {
          string interface_name = interface_it->first;
          
          DBusBindingBase *binding = find_binding(interface_name);
          if (binding == NULL)
            {
              throw DBusSystemException("Internal error, unknown interface");
            }
        
          str += "<interface name='" + interface_name + "'>\n";

          DBusIntrospect *table = binding->get_method_introspect();
          while (table->name != NULL)
            {
              str += string("<method name='") + table->name + "'>\n";

              const char *text = table->signature;
              while (*text)
                {
                  const char *name, *direction, *type;

                  direction = dbus_gettext(&text);
                  type = dbus_gettext(&text);
                  name = dbus_gettext(&text);
              
                  str += string("<arg name='") + name + "' type='" + type +"' direction='"+ direction +"'/>\n";
                }
              str += "</method>\n";
              table++;
            }
      
          table = binding->get_signal_introspect();
          while (table->name != NULL)
            {
              str += string("<signal name='") + table->name + "'>\n";

              const char *text = table->signature;
              while (*text)
                {
                  const char *name, *type;

                  type = dbus_gettext(&text);
                  name = dbus_gettext(&text);
              
                  str += string("<arg name='") + name + "' type='" + type +"'/>\n";
                }
              str += "</signal>\n";
              table++;
            }

          str += "</interface>\n";
        }
      str += "</node>\n";


      const char *cstr = str.c_str();

      DBusMessage *reply = dbus_message_new_method_return(message);

      dbus_message_append_args(reply, DBUS_TYPE_STRING, &cstr, DBUS_TYPE_INVALID);
      dbus_connection_send(connection, reply, NULL);
      dbus_message_unref(reply);
    
      return DBUS_HANDLER_RESULT_HANDLED;
    }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


DBusHandlerResult
DBus::handle_method(DBusConnection *connection, DBusMessage *message)
{
  string path = dbus_message_get_path(message);
  string interface_name = dbus_message_get_interface(message);
  string method = dbus_message_get_member(message);

  void *cobject = find_cobject(path, interface_name);
  if (cobject == NULL)
    {
      throw DBusUsageException("no such object");
    }
  
  DBusBindingBase *binding = find_binding(interface_name);
  if (binding == NULL)
    {
      throw DBusSystemException("No such binding");
    }

  DBusMessage *reply = binding->call(method, cobject, message);
  if (reply == NULL)
    {
      throw DBusUsageException("Call failure");
    }
  
  dbus_connection_send(connection, reply, NULL);
  dbus_message_unref(reply);
                
  return DBUS_HANDLER_RESULT_HANDLED;
}
