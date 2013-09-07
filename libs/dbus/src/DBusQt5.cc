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

#include "DBusQt5.hh"
#include "dbus/DBusBindingQt5.hh"
#include "dbus/DBusException.hh"
#include "dbus/IDBusWatch.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;

DBusQt5::Ptr
DBusQt5::create()
{
  return Ptr(new DBusQt5());
}

//! Construct a new D-BUS bridge
DBusQt5::DBusQt5()
  : connection(QDBusConnection::sessionBus()), watcher(this)
{
  watcher.setConnection(connection);
}


//! Destruct the D-BUS bridge
DBusQt5::~DBusQt5()
{
}


//! Initialize D-BUS bridge
void
DBusQt5::init()
{
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &DBusQt5::on_service_owner_changed);
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusQt5::on_service_registered);
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceUnregistered, this, &DBusQt5::on_service_unregistered);
}


//! Registers the specified service
void
DBusQt5::register_service(const std::string &service)
{
  connection.registerService(QString::fromStdString(service));
}


//! Registers the specified object path
void
DBusQt5::register_object_path(const string &object_path)
{
  bool success = connection.registerVirtualObject(QString::fromStdString(object_path), this);
  if (!success)
    {
      throw DBusException("Unable to register object");
    }
}


bool
DBusQt5::is_available() const
{
  return connection.isConnected();
}

bool
DBusQt5::is_running(const std::string &name) const
{
  QDBusConnectionInterface *i = connection.interface();
  QDBusReply<QString>	reply = i->serviceOwner(QString::fromStdString(name));
	return reply.isValid();
}

void
DBusQt5::watch(const std::string &name, IDBusWatch *cb)
{
  watcher.addWatchedService(QString::fromStdString(name));

  watched[name].callback = cb;
  watched[name].seen = false;
}

void
DBusQt5::unwatch(const std::string &name)
{
  watched.erase(name);
  watcher.removeWatchedService(QString::fromStdString(name));
}

QString
DBusQt5::introspect(const QString &path) const
{
  string str;

  ObjectCIter object_it = objects.find(path.toStdString());
  if (object_it != objects.end())
    {
      for (auto &interface_it : object_it->second) 
        {
          string interface_name = interface_it.first;
          DBusBindingQt5 *binding = dynamic_cast<DBusBindingQt5*>(find_binding(interface_name));
          if (binding != NULL)
            {
              str += binding->get_interface_introspect();
            }
        }
    }

  return str.c_str();
}

bool
DBusQt5::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
  bool success = false;
  string path = message.path().toStdString();
  string interface_name = message.interface().toStdString();;
  string method = message.member().toStdString();;

  try
    {
      void *cobject = find_object(path, interface_name);
      if (cobject != NULL)
        {
          DBusBindingQt5 *binding = dynamic_cast<DBusBindingQt5*>(find_binding(interface_name));
          if (binding != NULL)
            {
              success = binding->call(cobject, message, connection);
            }
        }
    }
  catch(DBusRemoteException &e)
    {
      std::cout << "DBusException: " << e.details() << std::endl;
      message.createErrorReply(QString::fromStdString(e.id()),
                               QString::fromStdString(e.details()));
      
    }

  return success;
}

void DBusQt5::on_service_owner_changed(const QString &name, const QString &oldowner, const QString &newowner)
{
}

void DBusQt5::on_service_registered(const QString &name)
{
  string service_name = name.toStdString();
  if (watched.find(service_name) != watched.end())
    {
      WatchData &w = watched[service_name];
      w.seen = true;
      w.callback->bus_name_presence(service_name, true);
    }
}

void DBusQt5::on_service_unregistered(const QString &name)
{
  string service_name = name.toStdString();
  if (watched.find(service_name) != watched.end())
    {
      WatchData &w = watched[service_name];
      if (w.seen)
        {
         w.callback->bus_name_presence(service_name, false);
        }
    }
}
