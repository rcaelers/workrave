// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

#include <vector>

#if defined(HAVE_DBUS_GIO)
#  include <glib.h>
#  include <gio/gio.h>
#endif

#include "session/IScreenLockMethod.hh"
#include "session/ISystemStateChangeMethod.hh"

class System
{
public:
  class SystemOperation
  {
  public:
    enum SystemOperationType
    {
      SYSTEM_OPERATION_NONE,
      SYSTEM_OPERATION_LOCK_SCREEN,
      SYSTEM_OPERATION_SHUTDOWN,
      SYSTEM_OPERATION_SUSPEND,
      SYSTEM_OPERATION_HIBERNATE,
      SYSTEM_OPERATION_SUSPEND_HYBRID,
    };

    // A simple, English language name of the operation
    // Not translated into native language here because
    // this class is not concerned with UI
    const char *name;
    SystemOperationType type;

    bool execute() const
    {
      return System::execute(type);
    }

    bool operator<(const SystemOperation &other) const
    {
      return this->type < other.type;
    }

  private:
    SystemOperation(const char *name, const SystemOperationType type)
      : name(name)
      , type(type){};
    friend class System;
  };

  static bool is_lockable()
  {
    return !lock_commands.empty();
  }
  static bool lock_screen();

  static std::vector<SystemOperation> get_supported_system_operations()
  {
    return supported_system_operations;
  }
  static bool execute(SystemOperation::SystemOperationType type);

  // display will not be owned by System,
  // the caller may free it after calling
  // this function
  static void init();
  static void clear();

private:
  static std::vector<IScreenLockMethod *> lock_commands;
  static std::vector<ISystemStateChangeMethod *> system_state_commands;
  static std::vector<SystemOperation> supported_system_operations;
#if defined(PLATFORM_OS_UNIX)

#  if defined(HAVE_DBUS_GIO)
  static void init_DBus();
  static void init_DBus_lock_commands();
  static inline bool add_DBus_lock_cmd(const char *dbus_name,
                                       const char *dbus_path,
                                       const char *dbus_interface,
                                       const char *dbus_lock_method,
                                       const char *dbus_method_to_check_existence);

  static void add_DBus_system_state_command(ISystemStateChangeMethod *method);
  static void init_DBus_system_state_commands();

  static GDBusConnection *session_connection;
  static GDBusConnection *system_connection;
#  endif

  static inline void add_cmdline_lock_cmd(const char *command_name, const char *parameters, bool async);
  static void init_cmdline_lock_commands(const char *display);
  static bool invoke(const gchar *command, bool async = false);
#endif // defined(PLATFORM_OS_UNIX)
#if defined(PLATFORM_OS_WINDOWS)
  static void init_windows_lock_commands();
  static void init_windows_system_state_commands();
#endif // PLATFORM_OS_WINDOWS
};

#endif // SYSTEM_HH
