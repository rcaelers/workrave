// DBusProxy-gio.cc --- support for simple DBUS method execution
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <iostream>

#include "DBusProxy-gio.hh"
#include "debug.hh"

bool
DBusProxy::init_with_connection(GDBusConnection *connection,
                                const char *name,
                                const char *object_path,
                                const char *interface_name,
                                GDBusProxyFlags flags_in)
{
  TRACE_ENTRY_PAR(name);
  this->flags = flags_in;
  proxy = g_dbus_proxy_new_sync(connection, flags, nullptr, name, object_path, interface_name, nullptr, &error);

  if (error != nullptr)
    {
      TRACE_MSG("Error: {}", error->message);
      return false;
    }
  return true;
}

bool
DBusProxy::init(GBusType bus_type,
                const char *name,
                const char *object_path,
                const char *interface_name,
                GDBusProxyFlags flags_in)
{
  TRACE_ENTRY_PAR(name);
  this->flags = flags_in;
  error = nullptr;
  proxy = g_dbus_proxy_new_for_bus_sync(bus_type, flags, nullptr, name, object_path, interface_name, nullptr, &error);

  if (error != nullptr)
    {
      TRACE_MSG("Error: {}", error->message);
      return false;
    }
  return true;
}

// Consumes (=deletes) method_parameters if it is floating
// method_result may be null, in this case the result of the method is ignored
bool
DBusProxy::call_method(const char *method_name, GVariant *method_parameters, GVariant **method_result)
{
  TRACE_ENTRY_PAR(method_name);
  if (proxy == nullptr)
    return false;

  if (error != nullptr)
    {
      g_error_free(error);
      error = nullptr;
    }

  GVariant *result = g_dbus_proxy_call_sync(proxy, method_name, method_parameters, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

  if (method_result == nullptr)
    {
      if (result != nullptr)
        {
          g_variant_unref(result);
          result = nullptr;
        }
    }
  else
    {
      if (error != nullptr)
        {
          *method_result = nullptr;
          if (result != nullptr)
            {
              g_variant_unref(result);
              result = nullptr;
            }
        }
      else
        {
          *method_result = result;
        }
    }

  if (error != nullptr)
    {
      TRACE_VAR(error->message);
      return false;
    }

  return true;
}

// Consumes (=deletes) method_parameters if it is floating
bool
DBusProxy::call_method_asynch_no_result(const char *method_name, GVariant *method_parameters)
{
  TRACE_ENTRY_PAR(method_name);
  if (proxy == nullptr)
    return false;

  if (error != nullptr)
    {
      g_error_free(error);
      error = nullptr;
    }

  g_dbus_proxy_call(proxy, method_name, method_parameters, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);

  return true;
}
