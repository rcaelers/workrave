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

#ifndef WORKRAVE_DBUS_DBUSFREEDESKTOP_HH
#define WORKRAVE_DBUS_DBUSFREEDESKTOP_HH

#include <string>
#include <list>

#include <memory>

#include <dbus/dbus.h>

#include "dbus/IDBus.hh"
#include "dbus/DBusBindingFreeDesktop.hh"
#include "DBusGeneric.hh"

namespace workrave
{
  namespace dbus
  {
    class DBusFreeDesktop
      : public DBusGeneric
      , public IDBusPrivateFreeDesktop
    {
    public:
      typedef std::shared_ptr<DBusFreeDesktop> Ptr;

    public:
      DBusFreeDesktop();
      virtual ~DBusFreeDesktop();

      typedef DBusMessage *DBusSignal;

      virtual void init();
      virtual void register_service(const std::string &service);
      virtual void register_object_path(const std::string &object_path);
      virtual bool is_available() const;

      DBusConnection *conn()
      {
        return connection;
      }

    private:
      DBusHandlerResult dispatch_static(DBusConnection *connection, DBusMessage *message);

      static DBusHandlerResult dispatch_static(DBusConnection *connection, DBusMessage *message, void *user_data);

      DBusHandlerResult dispatch(DBusConnection *connection, DBusMessage *message);
      DBusHandlerResult handle_introspect(DBusConnection *connection, DBusMessage *message);
      DBusHandlerResult handle_method(DBusConnection *connection, DBusMessage *message);

      void *find_object(const std::string &path, const std::string &interface_name) const;
      void send(DBusMessage *msg) const;

      friend class DBusBinding;

    private:
      //! Connection to the DBus.
      DBusConnection *connection;
      GMainContext *context;
      GSource *queue;
      GSList *watches;
      GSList *timeouts;

      static GSourceFuncs queue_funcs;

      static gboolean queue_prepare(GSource *source, gint *timeout);
      static gboolean queue_check(GSource *source);
      static gboolean queue_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
      static gboolean watch_dispatch(GIOChannel *source, GIOCondition condition, gpointer user_data);
      static void watch_finalize(gpointer data);
      static void watch_free(void *data);
      static dbus_bool_t watch_add(DBusWatch *watch, void *data);
      static void watch_remove(DBusWatch *watch, void *data);
      static void watch_toggled(DBusWatch *watch, void *data);
      static gboolean timeout_dispatch(gpointer data);
      static void timeout_free(void *data);
      static dbus_bool_t timeout_add(DBusTimeout *timeout, void *data);
      static void timeout_remove(DBusTimeout *timeout, void *data);
      static void timeout_toggled(DBusTimeout *timeout, void *data);
      static void wakeup(void *data);

      void connection_setup(GMainContext *context);
    };
  } // namespace dbus
} // namespace workrave
#endif // WORKRAVE_DBUS_DBUSFREEDESKTOP_HH
