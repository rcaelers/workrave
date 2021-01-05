/*
 * SystemStateChangeConsolekit.cc
 *
 *  Created on: 16 lut 2014
 *      Author: mateusz
 */

#include "SystemStateChangeConsolekit.hh"

#ifdef HAVE_GLIB
#  include <glib.h>
#endif

#ifdef HAVE_STRING_H
#  include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <iostream>
#include "debug.hh"

const char *SystemStateChangeConsolekit::dbus_name = "org.freedesktop.ConsoleKit";

SystemStateChangeConsolekit::SystemStateChangeConsolekit(GDBusConnection *connection)
{
  TRACE_ENTER("SystemStateChangeConsolekit::SystemStateChangeConsolekit");
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
          TRACE_MSG(false);
          can_shutdown = false;
        }
      else
        {
          gboolean r2;
          g_variant_get(result, "(b)", &r2);
          g_variant_unref(result);
          TRACE_MSG(r2);
          can_shutdown = (r2 == TRUE);
        }
    }
  TRACE_EXIT();
}

bool
SystemStateChangeConsolekit::shutdown()
{
  TRACE_ENTER("SystemStateChangeConsolekit::shutdown");
  bool r = proxy.call_method("Stop", nullptr, nullptr);
  TRACE_RETURN(r);
  return r;
}
