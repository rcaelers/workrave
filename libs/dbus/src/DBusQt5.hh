// DBusQt5.hh --- DBUS interface
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

#ifndef WORKRAVE_DBUS_DBUSQT5_HH
#define WORKRAVE_DBUS_DBUSQT5_HH

#include <string>
#include <map>
#include <list>

#include <boost/shared_ptr.hpp>

#include <QtDBus/QtDBus>

#include "dbus/IDBus.hh"
#include "dbus/DBusBindingQt5.hh"
#include "DBusGeneric.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusQt5 : public DBusGeneric, public IDBusPrivateQt5, public QDBusVirtualObject
    {
    public:
      typedef boost::shared_ptr<DBusQt5> Ptr;

    public:
      static Ptr create();

      DBusQt5();
      virtual ~DBusQt5();

      // IDBus
      virtual void init();
      virtual void register_service(const std::string &service);
      virtual void register_object_path(const std::string &object_path);
      virtual bool is_available() const;
      virtual bool is_running(const std::string &name) const;
      virtual void watch(const std::string &name, IDBusWatch *cb);
      virtual void unwatch(const std::string &name);

      //! IDBusPrivateQt5
      virtual QDBusConnection get_connection() { return connection; }

      // QDBusVirtualObject
      virtual QString introspect(const QString &path) const;
      virtual bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection);
      
      void on_service_owner_changed(const QString & name, const QString & oldowner, const QString & newowner);
      void on_service_registered(const QString & name);
      void on_service_unregistered(const QString & name);
      
    private:
      struct WatchData
      {
        IDBusWatch *callback;
        bool seen;
      };
      
      typedef std::map<std::string, WatchData> Watched;
      typedef Watched::iterator WatchIter;
      typedef Watched::const_iterator WatchCIter;
    
      //! Connection to the DBus.
      QDBusConnection connection;

      //!
      QDBusServiceWatcher watcher;

      //!
      Watched watched;
    };
  }
}
#endif // WORKRAVE_DBUS_DBUSQT5_HH
