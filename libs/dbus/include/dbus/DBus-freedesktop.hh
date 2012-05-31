// DBus.hh --- DBUS interface
//
// Copyright (C) 2007, 2008, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef DBUSFREEDESKTOP_HH
#define DBUSFREEDESKTOP_HH

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

    void *find_object(const std::string &path, const std::string &interface_name) const;
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
}

#endif // DBUSFREEDESKTOP_HH
