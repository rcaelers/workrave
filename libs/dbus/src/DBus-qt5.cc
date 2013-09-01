// DBus.c
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"
#include <string.h>

#include <string>
#include <list>
#include <map>

#include "dbus/DBus-qt5.hh"
#include "dbus/DBusBinding-qt5.hh"
#include "dbus/DBusException.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;

DBus::Ptr
DBus::create()
{
  return Ptr(new DBus());
}

//! Construct a new D-BUS bridge
DBus::DBus()
  : connection(QDBusConnection::sessionBus()), owner(false)
{
  ;
}


//! Destruct the D-BUS bridge
DBus::~DBus()
{
}


//! Initialize D-BUS bridge
void
DBus::init()
{
}


//! Registers the specified service
void
DBus::register_service(const std::string &service)
{
  owner = connection.registerService(QString::fromStdString(service));
}


//! Registers the specified object path
void
DBus::register_object_path(const string &object_path)
{
  bool success = connection.registerVirtualObject(QString::fromStdString(object_path), this);
  if (!success)
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


//! Disconnect a D-DBUS object/interface to a C object
void
DBus::disconnect(const std::string &object_path, const std::string &interface_name)
{
  ObjectIter it = objects.find(object_path);
  if (it != objects.end())
    {
      Interfaces &interfaces = it->second;

      interfaces.erase(interface_name);
    }
}


//! Register an interface binding
void
DBus::register_binding(const std::string &name, DBusBindingBase *interface)
{
  bindings[name] = interface;
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


void
DBus::send(QDBusMessage *msg) const
{
  // dbus_connection_send(connection, msg, NULL);
  // dbus_message_unref(msg);
  // dbus_connection_flush(connection);
}


void *
DBus::find_object(const std::string &path, const std::string &interface_name) const
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
  return connection.isConnected();
}

bool
DBus::is_running(const std::string &name) const
{
  QDBusConnectionInterface *i = connection.interface();
  QDBusReply<QString>	reply = i->serviceOwner(QString::fromStdString(name));
	return reply.isValid();
}

QString
DBus::introspect(const QString &path) const
{
  string str;

  ObjectCIter object_it = objects.find(path.toStdString());
  if (object_it != objects.end())
    {
      for (auto &interface_it : object_it->second) 
        {
          string interface_name = interface_it.first;

          DBusBindingBase *binding = find_binding(interface_name);
          if (binding == NULL)
            {
              throw DBusSystemException("Internal error, unknown interface");
            }

          str += binding->get_interface_introspect();
        }
    }

  return str.c_str();
}

bool
DBus::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
  bool success = false;
  string path = message.path().toStdString();
  string interface_name = message.interface().toStdString();;
  string method = message.member().toStdString();;

  void *cobject = find_object(path, interface_name);
  if (cobject != NULL)
    {
      DBusBindingBase *binding = find_binding(interface_name);
      if (binding != NULL)
        {
          success = binding->call(cobject, message, connection);
        }
    }

  return success;
}
