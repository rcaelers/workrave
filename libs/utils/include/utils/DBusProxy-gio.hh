// DBusProxy-gio.hh --- support for simple DBUS method execution
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
// All rights reserved.
// It is possible that it uses some code and ideas from the KShutdown utility:
//              file src/actions/lock.cpp
//              Copyright (C) 2009  Konrad Twardowski
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
//

#ifndef DBUSPROXYGIO_HH
#define DBUSPROXYGIO_HH

#include <glib.h>
#include <gio/gio.h>

class DBusProxy
{
private:
  GDBusProxy *proxy{nullptr};
  GError *error{nullptr};
  GDBusProxyFlags flags{G_DBUS_PROXY_FLAGS_NONE};

public:
  DBusProxy() = default;
  ~DBusProxy()
  {
    clear();
  }

  bool init(GBusType bus_type,
            const char *name,
            const char *object_path,
            const char *interface_name,
            GDBusProxyFlags flags_in = static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                                    | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS));

  bool init_with_connection(GDBusConnection *connection,
                            const char *name,
                            const char *object_path,
                            const char *interface_name,
                            GDBusProxyFlags flags_in = static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                                                    | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS));

  // Consumes (=deletes) method_parameters if it is floating
  // method_result may be null, in this case the result of the method is ignored
  bool call_method(const char *method_name, GVariant *method_parameters, GVariant **method_result);

  // Calls method asynchronously and does not accept result (no callback will be run)
  // Consumes (=deletes) method_parameters if it is floating
  bool call_method_asynch_no_result(const char *method_name, GVariant *method_parameters);

  void clear()
  {
    if (error != nullptr)
      {
        g_error_free(error);
        error = nullptr;
      }
    if (proxy != nullptr)
      {
        g_object_unref(proxy);
        proxy = nullptr;
      }
  }

  bool is_valid()
  {
    return proxy != nullptr;
  }

  gchar *get_last_error_message()
  {
    if (error == nullptr)
      {
        return nullptr;
      }

    return error->message;
  }
};

#endif
