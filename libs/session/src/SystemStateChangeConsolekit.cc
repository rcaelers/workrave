/*
 * SystemStateChangeConsolekit.cc
 *
 *  Created on: 16 lut 2014
 *      Author: mateusz
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "SystemStateChangeConsolekit.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

#include <cstring>
#include <strings.h>
#include <iostream>

#include "debug.hh"

const char *SystemStateChangeConsolekit::dbus_name = "org.freedesktop.ConsoleKit";

SystemStateChangeConsolekit::SystemStateChangeConsolekit(GDBusConnection *connection)
{
  TRACE_ENTRY();
  proxy.init_with_connection(connection,
                             dbus_name,
                             "/org/freedesktop/ConsoleKit/Manager",
                             "org.freedesktop.ConsoleKit.Manager",
                             static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS
                                                          | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START));

  if (!proxy.is_valid())
    {
      can_shutdown = false;
    }
  else
    {
      GVariant *result;
      if (!proxy.call_method("CanStop", nullptr, &result))
        {
          TRACE_VAR(false);
          can_shutdown = false;
        }
      else
        {
          gboolean r2;
          g_variant_get(result, "(b)", &r2);
          g_variant_unref(result);
          TRACE_VAR(r2);
          can_shutdown = (r2 == TRUE);
        }
    }
}

bool
SystemStateChangeConsolekit::shutdown()
{
  TRACE_ENTRY();
  bool r = proxy.call_method("Stop", nullptr, nullptr);
  TRACE_VAR(r);
  return r;
}
