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

#ifndef WORKRAVE_DBUS_DBUSQT_HH
#define WORKRAVE_DBUS_DBUSQT_HH

#include <string>
#include <map>
#include <list>

#include <memory>

#include <QtDBus/QtDBus>

#include "dbus/IDBus.hh"
#include "dbus/DBusBindingQt.hh"
#include "DBusGeneric.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusQt
      : public DBusGeneric
      , public IDBusPrivateQt
      , public QDBusVirtualObject
    {
    public:
      typedef std::shared_ptr<DBusQt> Ptr;

    public:
      DBusQt();
      ~DBusQt() override = default;

      // IDBus
      void init() override;
      void register_service(const std::string &service, IDBusWatch *cb = nullptr) override;
      void register_object_path(const std::string &object_path) override;
      bool is_available() const override;
      bool is_running(const std::string &name) const override;
      void watch(const std::string &name, IDBusWatch *cb) override;
      void unwatch(const std::string &name) override;

      //! IDBusPrivateQt
      QDBusConnection get_connection() override
      {
        return connection;
      }

      // QDBusVirtualObject
      QString introspect(const QString &path) const override;
      bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;

      void on_service_owner_changed(const QString &name, const QString &oldowner, const QString &newowner);
      void on_service_registered(const QString &name);
      void on_service_unregistered(const QString &name);

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
  } // namespace dbus
} // namespace workrave
#endif // WORKRAVE_DBUS_DBUSQT_HH
