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

#include "DBusProxy-gio.hh"
#include <iostream>

  bool DBusProxy::init_with_connection(GDBusConnection *connection, const char *name, const char *object_path, 
              const char *interface_name, GDBusProxyFlags flags_in)
  {
    TRACE_ENTER_MSG("DBus_proxy::init_with_connection", name);
    this->flags = flags_in;
    proxy = g_dbus_proxy_new_sync(connection, 
                                       flags,
                                       NULL,
                                       name,
                                       object_path,
                                       interface_name,
                                       NULL,
                                       &error);

    if (error != NULL)
      {
        TRACE_MSG("Error: " << error->message);
        return false;
      }
    TRACE_EXIT();
    return true;
  }

  bool DBusProxy::init(GBusType bus_type, const char *name, const char *object_path, 
              const char *interface_name, GDBusProxyFlags flags_in)
  {
    TRACE_ENTER_MSG("DBus_proxy::init", name);
    this->flags = flags_in;
    error = NULL;
    proxy = g_dbus_proxy_new_for_bus_sync(bus_type,
                                               flags,
                                               NULL,
                                               name,
                                               object_path,
                                               interface_name,
                                               NULL,
                                               &error);

    if (error != NULL)
      {
        TRACE_MSG("Error: " << error->message);
        return false;
      }
    TRACE_EXIT();
    return true;
  }


  //Consumes (=deletes) method_parameters if it is floating
  //method_result may be null, in this case the result of the method is ignored
  bool DBusProxy::call_method(const char *method_name, GVariant *method_parameters, GVariant **method_result)
  {
    TRACE_ENTER_MSG("DBus_proxy::call_method", method_name);
    if (proxy == NULL)
      return false;
    
    if (error != NULL)
      {
        g_error_free(error);
        error = NULL;
      }

    GVariant *result = g_dbus_proxy_call_sync(proxy, method_name,
                                              method_parameters,
                                              G_DBUS_CALL_FLAGS_NONE,
                                              -1,
                                              NULL,
                                              &error);

    if (method_result == NULL)
      {
        if (result != NULL)
          { 
            g_variant_unref(result);
            result = NULL;
          }
      }
    else
      {
        if (error != NULL)
          {
            *method_result = NULL;
            if (result != NULL)
              { 
                g_variant_unref(result);
                result = NULL;
              }
          } 
        else 
          {
            *method_result = result;
          }
      }

    if (error != NULL)
      {
        TRACE_RETURN(error->message);
        return false;
      }

    TRACE_EXIT();
    return true;
  }

  bool DBusProxy::give_list_of_all_available_services(GDBusConnection* connection, std::set<std::string> *services)
  {
    TRACE_ENTER("DBusProxy::give_list_of_all_available_services");
    DBusProxy proxy;

    if (!proxy.init_with_connection(connection,
        "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus"))
      {
        TRACE_RETURN(false);
        return false;
      }

    GVariant *method_result = NULL;
    if (!proxy.call_method("ListNames", NULL, &method_result))
      {
        TRACE_RETURN(false);
        return false;
      }

    services->clear();
    bool ret = get_all_strings_from_gvariant(method_result, services);

    g_variant_unref(method_result);

    TRACE_RETURN(ret);
    return ret;
  }

  //Based on an example in https://developer.gnome.org/glib/2.32/glib-GVariant.html#g-variant-iter-next-value
  bool DBusProxy::get_all_strings_from_gvariant(GVariant *container, std::set<std::string> *services)
  {
    GVariantIter iter;
    GVariant *child;

    g_variant_iter_init (&iter, container);
    while ((child = g_variant_iter_next_value (&iter)))
      {
        bool r = true;

        if (g_variant_is_container (child))
          {
            r = get_all_strings_from_gvariant(child, services);
          }
        else
          {
            if (!g_variant_type_equal(g_variant_get_type(child), G_VARIANT_TYPE_STRING))
              {
                r = false;
                std::cerr << "ERROR: org.freedesktop.DBus.ListNames returned data in strange format" << std::endl;
              }
            else
              {
                const char *cstr = g_variant_get_string(child, NULL);
                //do not return unique connection names
                if (cstr[0] != ':')
                  {
                    services->insert(std::string(cstr));
                  }
              }
          }

        g_variant_unref (child);
        if (!r)
          {
            return false;
          }
      }

    return true;
  }
