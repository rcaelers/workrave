// SystemStateChangeLogind.cc -- shutdown/suspend/hibernate using systemd-logind
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

#include "SystemStateChangeLogind.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

#include <cstring>
#include <strings.h>

#include <iostream>
#include "debug.hh"

const char *SystemStateChangeLogind::dbus_name = "org.freedesktop.login1";

SystemStateChangeLogind::SystemStateChangeLogind(GDBusConnection *connection)
{
  TRACE_ENTRY();
  proxy.init_with_connection(connection,
                             dbus_name,
                             "/org/freedesktop/login1",
                             "org.freedesktop.login1.Manager",
                             static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START));

  if (!proxy.is_valid())
    {
      can_shutdown = false;
      can_suspend = false;
      can_hibernate = false;
      can_suspend_hybrid = false;
    }
  else
    {
      // CanPowerOff(), CanReboot(), CanSuspend(), CanHibernate(), CanHybridSleep()
      can_shutdown = check_method("CanPowerOff");
      can_suspend = check_method("CanSuspend");
      can_hibernate = check_method("CanHibernate");
      can_suspend_hybrid = check_method("CanHybridSleep");
    }
}

bool
SystemStateChangeLogind::check_method(const char *method_name)
{
  TRACE_ENTRY_PAR(method_name);

  bool ret;
  GVariant *result;
  if (!proxy.call_method(method_name, nullptr, &result))
    {
      TRACE_VAR(false);
      return false;
    }

  gchar *cresult;
  g_variant_get(result, "(s)", &cresult);
  g_variant_unref(result);
  result = nullptr;
  if (cresult == nullptr)
    {
      TRACE_VAR(false);
      return false;
    }

  TRACE_MSG("Method returned: {}", cresult);
  if (strcmp(cresult, "yes") == 0)
    {
      ret = true;
    }
  else
    {
      ret = false;
    }

  g_free(cresult);
  cresult = nullptr;

  TRACE_VAR(ret);
  return ret;
}

bool
SystemStateChangeLogind::execute(const char *method_name)
{
  TRACE_ENTRY_PAR(method_name);

  // We do not want PolicyKit to ask for credentials
  bool ret = proxy.call_method(method_name, g_variant_new("(b)", false), nullptr);

  TRACE_VAR(ret);
  return ret;
}
