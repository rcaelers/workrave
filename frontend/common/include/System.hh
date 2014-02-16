// System.hh
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifndef SYSTEM_HH
#define SYSTEM_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#endif

#include <vector>

#if defined(HAVE_DBUS_GIO)
#include <glib.h>
#include <gio/gio.h>
#endif

#include "IScreenLockMethod.hh"
#include "ISystemStateChangeMethod.hh"

class System
{
public:
  static bool is_lockable() { return !lock_commands.empty(); };
  static bool is_shutdown_supported();
  static void lock();
  static void shutdown();

  //display will not be owned by System,
  //the caller may free it after calling
  //this function
  static void init(
#if defined(PLATFORM_OS_UNIX)
                   const char *display
#endif
                   );
  static void clear();

private:
  static std::vector<IScreenLockMethod *> lock_commands;
  static std::vector<ISystemStateChangeMethod *> system_state_commands;

#if defined(PLATFORM_OS_UNIX)

#ifdef HAVE_DBUS_GIO
  static void init_DBus();
  static void init_DBus_lock_commands();
  static inline bool add_DBus_lock_cmd(
      const char *dbus_name, const char *dbus_path, const char *dbus_interface,
      const char *dbus_lock_method, const char *dbus_method_to_check_existence);

  static void add_DBus_system_state_command(
      ISystemStateChangeMethod *method);
  static void init_DBus_system_state_commands();

  static GDBusConnection* session_connection;
  static GDBusConnection* system_connection;
#endif

  static bool shutdown_supported;
  static inline void add_cmdline_lock_cmd(
        const char *command_name, const char *parameters, bool async);
  static void init_cmdline_lock_commands(const char *display);
  static bool invoke(const gchar* command, bool async = false);
#endif //defined(PLATFORM_OS_UNIX)

#if defined(PLATFORM_OS_WIN32)
  static bool shutdown_helper(bool for_real);
  static bool shutdown_supported;
#endif
};

#endif // SYSTEM_HH
