// SystemStateChangeUPower.cc -- shutdown/suspend/hibernate using systemd-logind
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
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
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "SystemStateChangeUPower.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

#include <cstring>

#include <iostream>
#include "debug.hh"

const char *SystemStateChangeUPower::dbus_name = "org.freedesktop.UPower";

SystemStateChangeUPower::SystemStateChangeUPower(GDBusConnection *connection)
{
  TRACE_ENTRY();
  proxy.init_with_connection(connection,
                             dbus_name,
                             "/org/freedesktop/UPower",
                             "org.freedesktop.UPower",
                             static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START));

  property_proxy.init_with_connection(connection,
                                      "org.freedesktop.UPower",
                                      "/org/freedesktop/UPower",
                                      "org.freedesktop.DBus.Properties");

  if (!proxy.is_valid() || !property_proxy.is_valid())
    {
      can_suspend = false;
      can_hibernate = false;
    }
  else
    {
      can_suspend = check_method("SuspendAllowed") && check_property("CanSuspend");
      can_hibernate = check_method("HibernateAllowed") && check_property("CanHibernate");
    }
}

bool
SystemStateChangeUPower::check_method(const char *method_name)
{
  TRACE_ENTRY_PAR(method_name);

  GVariant *result;
  if (!proxy.call_method(method_name, nullptr, &result))
    {
      TRACE_MSG("{} failed", method_name);
      TRACE_VAR(false);
      return false;
    }

  gboolean method_result;
  g_variant_get(result, "(b)", &method_result);

  g_variant_unref(result);

  TRACE_VAR(method_result);
  return method_result == TRUE;
}

bool
SystemStateChangeUPower::check_property(const char *property_name)
{
  TRACE_ENTRY_PAR(property_name);

  GVariant *result;
  bool r1;
  r1 = property_proxy.call_method("Get", g_variant_new("(ss)", "org.freedesktop.UPower", property_name), &result);

  if (!r1)
    {
      TRACE_MSG("{} failed", property_name);
      TRACE_VAR(false);
      return false;
    }

  GVariant *content;
  g_variant_get(result, "(v)", &content);
  if (content == nullptr)
    {
      return false;
    }

  gboolean prop_value;
  g_variant_get(content, "b", &prop_value);

  g_variant_unref(content);
  g_variant_unref(result);

  TRACE_VAR(prop_value);
  return (prop_value == TRUE);
}

bool
SystemStateChangeUPower::execute(const char *method_name)
{
  TRACE_ENTRY_PAR(method_name);

  bool ret = proxy.call_method_asynch_no_result(method_name, nullptr);

  TRACE_VAR(ret);
  return ret;
}
