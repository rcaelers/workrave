// SystemLockDBus.cc -- support for locking the system using DBus
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

#include "ScreenLockDBus.hh"

#include "debug.hh"

ScreenLockDBus::ScreenLockDBus(GDBusConnection *connection,
                               const char *dbus_name,
                               const char *dbus_path,
                               const char *dbus_interface,
                               const char *dbus_lock_method,
                               const char *dbus_method_to_check_existence)
  : dbus_lock_method(dbus_lock_method)
{
  TRACE_ENTRY_PAR(dbus_name);

  // We do not allow autospawning services
  bool r = proxy.init_with_connection(connection,
                                      dbus_name,
                                      dbus_path,
                                      dbus_interface,
                                      static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES
                                                                   | G_DBUS_PROXY_FLAGS_DO_NOT_CONNECT_SIGNALS
                                                                   | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START));

  if (r && dbus_method_to_check_existence != nullptr)
    {
      r = proxy.call_method(dbus_method_to_check_existence, nullptr, nullptr);
    }
  if (!r)
    {
      proxy.clear();
    }
}

bool
ScreenLockDBus::lock()
{
  TRACE_ENTRY_PAR(dbus_lock_method);
  return proxy.call_method_asynch_no_result(dbus_lock_method, nullptr);
}
