// Copyright (C) 2007 - 2013 Rob Caelers <robc@krandor.nl>
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
#  include "config.h"
#endif

#include "debug.hh"
#include <string.h>

#include <string>
#include <list>
#include <map>

#include "DBusQt.hh"
#include "dbus/DBusBindingQt.hh"
#include "dbus/DBusException.hh"
#include "dbus/IDBusWatch.hh"

using namespace std;
using namespace workrave;
using namespace workrave::dbus;

DBusQt::DBusQt()
  : connection(QDBusConnection::sessionBus())
  , watcher(this)
{
  watcher.setConnection(connection);
}

void
DBusQt::init()
{
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &DBusQt::on_service_owner_changed);
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceRegistered, this, &DBusQt::on_service_registered);
  QObject::connect(&watcher, &QDBusServiceWatcher::serviceUnregistered, this, &DBusQt::on_service_unregistered);
}

void
DBusQt::register_service(const std::string &service, IDBusWatch *cb)
{
  // TODO: use cb
  connection.registerService(QString::fromStdString(service));
}

void
DBusQt::register_object_path(const string &object_path)
{
  bool success = connection.registerVirtualObject(QString::fromStdString(object_path), this);
  if (!success)
    {
      throw DBusException("Unable to register object");
    }
}

bool
DBusQt::is_available() const
{

  return connection.isConnected();
}

bool
DBusQt::is_running(const std::string &name) const
{
  QDBusConnectionInterface *i = connection.interface();
  QDBusReply<QString> reply = i->serviceOwner(QString::fromStdString(name));
  return reply.isValid();
}

void
DBusQt::watch(const std::string &name, IDBusWatch *cb)
{
  watcher.addWatchedService(QString::fromStdString(name));

  watched[name].callback = cb;
  watched[name].seen = false;
}

void
DBusQt::unwatch(const std::string &name)
{
  watched.erase(name);
  watcher.removeWatchedService(QString::fromStdString(name));
}

QString
DBusQt::introspect(const QString &path) const
{
  string str;

  ObjectCIter object_it = objects.find(path.toStdString());
  if (object_it != objects.end())
    {
      for (auto &interface: object_it->second)
        {
          string interface_name = interface.first;
          DBusBindingQt *binding = dynamic_cast<DBusBindingQt *>(find_binding(interface_name));
          if (binding != nullptr)
            {
              str += binding->get_interface_introspect();
            }
        }
    }

  return str.c_str();
}

bool
DBusQt::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
  bool success = false;
  string path = message.path().toStdString();
  string interface_name = message.interface().toStdString();

  try
    {
      void *cobject = find_object(path, interface_name);
      if (cobject != nullptr)
        {
          DBusBindingQt *binding = dynamic_cast<DBusBindingQt *>(find_binding(interface_name));
          if (binding != nullptr)
            {
              success = binding->call(cobject, message, connection);
            }
        }
    }
  catch (DBusRemoteException &e)
    {
      e << object_info(path);
      message.createErrorReply(QString::fromStdString(e.error()), QString::fromStdString(e.diag()));
    }

  return success;
}

void
DBusQt::on_service_owner_changed(const QString &name, const QString &oldowner, const QString &newowner)
{
  (void)name;
  (void)oldowner;
  (void)newowner;
}

void
DBusQt::on_service_registered(const QString &name)
{
  string service_name = name.toStdString();
  if (watched.find(service_name) != watched.end())
    {
      WatchData &w = watched[service_name];
      w.seen = true;
      w.callback->bus_name_presence(service_name, true);
    }
}

void
DBusQt::on_service_unregistered(const QString &name)
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
