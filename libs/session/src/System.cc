// Copyright (C) 2002 - 2011 Rob Caelers & Raymond Penners
// Copyright (C) 2014 Mateusz Jończyk
// All rights reserved.
// Some lock commands are imported from the KShutdown utility:
//          http://kshutdown.sourceforge.net/
//          file src/actions/lock.cpp
//          Copyright (C) 2009  Konrad Twardowski
// Mateusz Jończyk has read source code of xflock4, lxlock and enlightenment_remote,
// but that did not influence the code in any way except for getting simple info on how
// they work.
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

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include <algorithm>
#include <iostream>

#include "session/System.hh"
#include "debug.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "ScreenLockMacOS.hh"
#  include "ScreenLockCustomCommand.hh"
#  include "SystemStateChangeMacOS.hh"
#elif defined(PLATFORM_OS_UNIX)
#  include "utils/Platform.hh"
#  include "ScreenLockCommandline.hh"
#  include "ScreenLockCustomCommand.hh"

#  if defined(HAVE_DBUS_GIO)
#    include "ScreenLockDBus.hh"
#    include "SystemStateChangeConsolekit.hh"
#    include "SystemStateChangeLogind.hh"
#    include "SystemStateChangeUPower.hh"
#  endif
#endif // PLATFORM_OS_MACOS / PLATFORM_OS_UNIX

#if defined(PLATFORM_OS_WINDOWS)
#  include "W32Shutdown.hh"
#  include "W32LockScreen.hh"
#endif

std::vector<IScreenLockMethod *> System::lock_commands;
std::vector<std::string> System::lock_command_ids;
std::vector<std::string> System::lock_command_labels;
std::vector<ISystemStateChangeMethod *> System::system_state_commands;
std::vector<System::SystemOperation> System::supported_system_operations;

#if defined(PLATFORM_OS_UNIX) && defined(HAVE_DBUS_GIO)
GDBusConnection *System::session_connection = nullptr;
GDBusConnection *System::system_connection = nullptr;
#endif

#if defined(PLATFORM_OS_UNIX)
#  if defined(HAVE_DBUS_GIO)
void
System::init_DBus()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  session_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
  if (error != nullptr)
    {
      // it is rare and serious, so report it the user
      std::cerr << "Cannot establish connection to the session bus: " << error->message << std::endl;
      g_error_free(error);
      error = nullptr;
    }

  system_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
  if (error != nullptr)
    {
      std::cerr << "Cannot establish connection to the system bus: " << error->message << std::endl;
      g_error_free(error);
      error = nullptr;
    }
}

static const char *
dbus_name_to_label(const char *dbus_name)
{
  if (strcmp(dbus_name, "org.gnome.ScreenSaver") == 0)
    return "GNOME (DBus)";
  if (strcmp(dbus_name, "org.cinnamon.ScreenSaver") == 0)
    return "Cinnamon (DBus)";
  if (strcmp(dbus_name, "org.mate.ScreenSaver") == 0)
    return "MATE (DBus)";
  if (strcmp(dbus_name, "org.freedesktop.ScreenSaver") == 0)
    return "FreeDesktop (DBus)";
  if (strcmp(dbus_name, "org.kde.screensaver") == 0)
    return "KDE Screensaver (DBus)";
  if (strcmp(dbus_name, "org.kde.krunner") == 0)
    return "KDE KRunner (DBus)";
  if (strcmp(dbus_name, "org.kde.ksmserver") == 0)
    return "KDE Session Manager (DBus)";
  if (strcmp(dbus_name, "org.enlightenment.wm.service") == 0)
    return "Enlightenment (DBus)";
  return dbus_name;
}

bool
System::add_DBus_lock_cmd(const char *dbus_name,
                          const char *dbus_path,
                          const char *dbus_interface,
                          const char *dbus_lock_method,
                          const char *dbus_method_to_check_existence)
{
  TRACE_ENTRY_PAR(dbus_name);

  // I wish we could use std::move here
  IScreenLockMethod *lock_method = nullptr;
  lock_method = new ScreenLockDBus(session_connection,
                                   dbus_name,
                                   dbus_path,
                                   dbus_interface,
                                   dbus_lock_method,
                                   dbus_method_to_check_existence);
  if (!lock_method->is_lock_supported())
    {
      delete lock_method;
      lock_method = nullptr;
      TRACE_VAR(false);
      return false;
    }

  lock_commands.push_back(lock_method);
  lock_command_ids.push_back(std::string("dbus:") + dbus_name);
  lock_command_labels.push_back(dbus_name_to_label(dbus_name));
  TRACE_VAR(true);
  return true;
}

void
System::init_DBus_lock_commands()
{
  TRACE_ENTRY();
  if (session_connection != nullptr)
    {
      //  Unity:
      //    - Gnome screensaver API + gnome-screensaver-command works,
      //    - is going to decrease dependence and use of GNOME:
      //      https://blueprints.launchpad.net/unity/+spec/client-1311-unity7-lockscreen
      //  GNOME
      //      https://people.gnome.org/~mccann/gnome-screensaver/docs/gnome-screensaver.html#gs-method-GetSessionIdle
      //    - Gnome is now implementing the Freedesktop API, but incompletely:
      //      https://bugzilla.gnome.org/show_bug.cgi?id=689225
      //      (look for "unimplemented" in the patch), the lock method is still unipmlemented
      //    - therefore it is required to check the gnome API first.
      // WORKS: Ubuntu 12.04: GNOME 3 fallback, Unity
      add_DBus_lock_cmd("org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver", "Lock", "GetActive");

      //  Cinnamon:   https://github.com/linuxmint/cinnamon-screensaver/blob/master/doc/dbus-interface.html
      //    Same api as GNOME, but with different name,
      add_DBus_lock_cmd("org.cinnamon.ScreenSaver", "/org/cinnamon/ScreenSaver", "org.cinnamon.ScreenSaver", "Lock", "GetActive");

      //  Mate: https://github.com/mate-desktop/mate-screensaver/blob/master/doc/dbus-interface.xml
      //  Like GNOME
      add_DBus_lock_cmd("org.mate.ScreenSaver", "/org/mate/ScreenSaver", "org.mate.ScreenSaver", "Lock", "GetActive");

      // The FreeDesktop API - the most important and most widely supported
      //    LXDE:  https://github.com/lxde/lxqt-powermanagement/blob/master/idleness/idlenesswatcherd.cpp
      //    KDE:
      //      https://projects.kde.org/projects/kde/kde-workspace/repository/revisions/master/entry/ksmserver/screenlocker/dbus/org.freedesktop.ScreenSaver.xml
      //      - there have been claims that this does not work in some installations, but I was unable to find
      //      any traces of this in git:
      //                        http://forum.kde.org/viewtopic.php?f=67&t=111003
      //                      It was probably due to some upgrade problems (and/or a bug in KDE),
      //                      because in fresh OpenSuse 12.3 (from LiveCD) this works correctly.
      //    Razor-QT: https://github.com/Razor-qt/razor-qt/blob/master/razorqt-screenlocker/src/razorscreenlocker.cpp
      //
      //    The Freedesktop API that these DEs are implementing is being redrafted:
      //      http://people.freedesktop.org/~hadess/idle-inhibition-spec/
      //      http://lists.freedesktop.org/pipermail/xdg/2012-November/012577.html
      //      http://lists.freedesktop.org/pipermail/xdg/2013-September/012875.html
      //
      //    the Lock method there is being removed (and not replaced with anything else).
      //    Probably the DEs will support these APIs in the future in order not to break other software.

      // Is only partially implemented by GNOME, so GNOME has to go before
      // Works correctly on KDE4 (Ubuntu 12.04)
      add_DBus_lock_cmd("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Lock", "GetActive");

      //  KDE - old screensaver API - partially verified both
      add_DBus_lock_cmd("org.kde.screensaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Lock", "GetActive");
      add_DBus_lock_cmd("org.kde.krunner", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Lock", "GetActive");

      //              - there some accounts that when org.freedesktop.ScreenSaver does not work, this works:
      //                      qdbus org.kde.ksmserver /ScreenSaver Lock
      //                      but it is probably a side effect of the fact that implementation of org.kde.ksmserver
      //                      is in the same process as of org.freedesktop.ScreenSaver
      add_DBus_lock_cmd("org.kde.ksmserver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "Lock", "GetActive");

      // EFL:
      add_DBus_lock_cmd("org.enlightenment.wm.service",
                        "/org/enlightenment/wm/RemoteObject",
                        "org.enlightenment.wm.Desktop",
                        "Lock",
                        "GetVirtualCount");
    }
}

void
System::add_DBus_system_state_command(ISystemStateChangeMethod *method)
{
  TRACE_ENTRY();
  if (method->canDoAnything())
    {
      system_state_commands.push_back(method);
      TRACE_MSG("DBus service is useful");
    }
  else
    {
      delete method;
      TRACE_MSG("DBus service is useless");
    }
}

void
System::init_DBus_system_state_commands()
{
  TRACE_ENTRY();
  if (system_connection != nullptr)
    {
      // These three DBus interfaces are too diverse
      // to implement support for them in one class
      // Logind is the future so it goes first
      add_DBus_system_state_command(new SystemStateChangeLogind(system_connection));

      add_DBus_system_state_command(new SystemStateChangeUPower(system_connection));

      // ConsoleKit is deprecated so goes last
      add_DBus_system_state_command(new SystemStateChangeConsolekit(system_connection));

      // Other interfaces:
      //  GNOME:
      //    - there is GNOME Session API:
      //      https://git.gnome.org/browse/gnome-session/tree/gnome-session/org.gnome.SessionManager.xml
      //      But shutdown/reboot require confirmation, so this is unusable to us,
      //
      //  KDE:
      //    - there is some support, but probably not worth implementing it
      //    http://askubuntu.com/questions/1871/how-can-i-safely-shutdown-reboot-logout-kde-from-the-command-line

      //  Windows:
      //  http://www.programmingsimplified.com/c-program-shutdown-computer
      //    winxp
      //        system("C:\\WINDOWS\\System32\\shutdown -s");
      //    win7
      //        system("C:\\WINDOWS\\System32\\shutdown /s");
    }
}

#  endif // HAVE_DBUS_GIO

void
System::add_cmdline_lock_cmd(const char *command_name, const char *parameters, bool async)
{
  TRACE_ENTRY_PAR(command_name);
  IScreenLockMethod *lock_method = nullptr;
  lock_method = new ScreenLockCommandline(command_name, parameters, async);
  if (!lock_method->is_lock_supported())
    {
      delete lock_method;
      lock_method = nullptr;
      TRACE_VAR(false);
    }
  else
    {
      lock_commands.push_back(lock_method);
      lock_command_ids.push_back(std::string("cmd:") + command_name);
      lock_command_labels.push_back(command_name);
      TRACE_VAR(true);
    }
}

void
System::init_cmdline_lock_commands(const char *display)
{
  TRACE_ENTRY_PAR(display);

  // Works: XFCE, i3, LXDE
  add_cmdline_lock_cmd("gnome-screensaver-command", "--lock", false);
  add_cmdline_lock_cmd("mate-screensaver-command", "--lock", false);
  add_cmdline_lock_cmd("enlightenment_remote", "-desktop-lock", false);
  add_cmdline_lock_cmd("xdg-screensaver", "lock", false);

  if (display != nullptr)
    {
      char *cmd = g_strdup_printf("-display \"%s\" -lock", display);
      add_cmdline_lock_cmd("xscreensaver-command", cmd, false);
      g_free(cmd);
      cmd = nullptr;
    }
  else
    {
      add_cmdline_lock_cmd("xscreensaver-command", "-lock", false);
    }

  // these two may call slock, which may be not user-friendly
  // add_cmdline_lock_cmd("xflock4", NULL, true);
  // add_cmdline_lock_cmd("lxlock", NULL, true);

  if (display != nullptr)
    {
      char *cmd = g_strdup_printf("-display \"%s\"", display);
      add_cmdline_lock_cmd("xlock", cmd, true);
      g_free(cmd);
      cmd = nullptr;
    }
  else
    {
      add_cmdline_lock_cmd("xlock", nullptr, true);
    }
}

#endif // PLATFORM_OS_UNIX

#if defined(PLATFORM_OS_WINDOWS)

void
System::init_windows_lock_commands()
{
  TRACE_ENTRY();
  IScreenLockMethod *winLock = new W32LockScreen();
  if (winLock->is_lock_supported())
    {
      lock_commands.push_back(winLock);
      lock_command_ids.push_back("win32:lock");
      lock_command_labels.push_back("Windows Lock Screen");
    }
  else
    {
      delete winLock;
      winLock = nullptr;
    }
}

void
System::init_windows_system_state_commands()
{
  TRACE_ENTRY();
}

#endif // PLATFORM_OS_WINDOWS

void
System::set_custom_lock_command(const std::string &cmd)
{
#if defined(PLATFORM_OS_MACOS) || defined(PLATFORM_OS_UNIX)
  auto it = std::find(lock_command_ids.begin(), lock_command_ids.end(), "custom");
  if (it != lock_command_ids.end())
    {
      size_t idx = static_cast<size_t>(it - lock_command_ids.begin());
      delete lock_commands[idx];
      lock_commands.erase(lock_commands.begin() + static_cast<ptrdiff_t>(idx));
      lock_command_ids.erase(lock_command_ids.begin() + static_cast<ptrdiff_t>(idx));
      lock_command_labels.erase(lock_command_labels.begin() + static_cast<ptrdiff_t>(idx));
    }
  if (!cmd.empty())
    {
      lock_commands.push_back(new ScreenLockCustomCommand(cmd));
      lock_command_ids.push_back("custom");
      lock_command_labels.push_back("Custom command");
    }
#else
  (void)cmd;
#endif
}

#if defined(PLATFORM_OS_MACOS)
void
System::init_macos_lock_commands()
{
  lock_commands.push_back(new ScreenLockMacOS());
  lock_command_ids.push_back("macos:lock");
  lock_command_labels.push_back("macOS Lock Screen");
}

void
System::init_macos_system_state_commands()
{
  system_state_commands.push_back(new SystemStateChangeMacOS());
}
#endif

bool
System::lock_screen()
{
  TRACE_ENTRY();
  for (auto &lock_command: lock_commands)
    {
      if (lock_command->lock())
        {
          TRACE_VAR(true);
          return true;
        }
    }

  TRACE_VAR(false);
  return false;
}

bool
System::lock_screen_by_id(const std::string &id)
{
  TRACE_ENTRY();
  if (!id.empty())
    {
      for (size_t i = 0; i < lock_commands.size(); i++)
        {
          if (lock_command_ids[i] == id)
            {
              return lock_commands[i]->lock();
            }
        }
    }
  return lock_screen();
}

bool
System::is_shutdownable()
{
  for (auto &cmd: system_state_commands)
    {
      if (cmd->canShutdown())
        {
          return true;
        }
    }
  return false;
}

bool
System::is_sleepable()
{
  for (auto &cmd: system_state_commands)
    {
      if (cmd->canSuspend() || cmd->canHibernate() || cmd->canSuspendHybrid())
        {
          return true;
        }
    }
  return false;
}

std::vector<System::LockMethodInfo>
System::get_lock_methods()
{
  std::vector<LockMethodInfo> result;
  for (size_t i = 0; i < lock_commands.size(); i++)
    {
      result.push_back({lock_command_ids[i], lock_command_labels[i]});
    }
  return result;
}

std::vector<System::SystemOperation>
System::get_sleep_operations()
{
  std::vector<SystemOperation> ops;
  for (const auto &op: supported_system_operations)
    {
      if (op.type == SystemOperation::SYSTEM_OPERATION_SUSPEND
          || op.type == SystemOperation::SYSTEM_OPERATION_HIBERNATE
          || op.type == SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID)
        {
          ops.push_back(op);
        }
    }
  return ops;
}

bool
System::execute(SystemOperation::SystemOperationType type)
{
  TRACE_ENTRY();
  if (type == SystemOperation::SYSTEM_OPERATION_NONE)
    {
      return false;
    }

  if (type == SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN)
    {
      return lock_screen();
    }

  for (auto &system_state_command: system_state_commands)
    {
      bool ret = false;
      switch (type)
        {
        case SystemOperation::SYSTEM_OPERATION_SHUTDOWN:
          ret = (system_state_command->shutdown());
          break;
        case SystemOperation::SYSTEM_OPERATION_SUSPEND:
          ret = (system_state_command->suspend());
          break;
        case SystemOperation::SYSTEM_OPERATION_HIBERNATE:
          ret = (system_state_command->hibernate());
          break;
        case SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
          ret = (system_state_command->suspendHybrid());
          break;
        default:
          throw "System::execute: Unknown system operation";
        }
      if (ret)
        {
          TRACE_VAR(true);
          return true;
        }
    }

  TRACE_VAR(false);
  return false;
}

void
System::init()
{
  TRACE_ENTRY();

#if defined(PLATFORM_OS_MACOS)
  init_macos_lock_commands();
  init_macos_system_state_commands();
#elif defined(PLATFORM_OS_UNIX)
  {
    std::string display = workrave::utils::Platform::get_default_display_name();
#  if defined(HAVE_DBUS_GIO)
    init_DBus();
    init_DBus_lock_commands();
    init_DBus_system_state_commands();
#  endif
    init_cmdline_lock_commands(display.c_str());
  }
#elif defined(PLATFORM_OS_WINDOWS)
  init_windows_lock_commands();
  init_windows_system_state_commands();
#endif

  if (is_lockable())
    {
      supported_system_operations.push_back(SystemOperation("Lock", SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN));
    }

  for (auto &system_state_command: system_state_commands)
    {
      if (system_state_command->canShutdown())
        {
          supported_system_operations.push_back(SystemOperation("Shutdown", SystemOperation::SYSTEM_OPERATION_SHUTDOWN));
        }
      if (system_state_command->canSuspend())
        {
          supported_system_operations.push_back(SystemOperation("Suspend", SystemOperation::SYSTEM_OPERATION_SUSPEND));
        }
      if (system_state_command->canHibernate())
        {
          supported_system_operations.push_back(SystemOperation("Hibernate", SystemOperation::SYSTEM_OPERATION_HIBERNATE));
        }
      if (system_state_command->canSuspendHybrid())
        {
          supported_system_operations.push_back(
            SystemOperation("Suspend hybrid", SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID));
        }
    }

  std::sort(supported_system_operations.begin(), supported_system_operations.end());
}

void
System::clear()
{
  for (auto &lock_command: lock_commands)
    {
      delete lock_command;
    }
  lock_commands.clear();
  lock_command_ids.clear();
  lock_command_labels.clear();

  for (auto &system_state_command: system_state_commands)
    {
      delete system_state_command;
    }
  system_state_commands.clear();

#if defined(PLATFORM_OS_UNIX) && defined(HAVE_DBUS_GIO)
  // we shouldn't call g_dbus_connection_close_sync here:
  // http://comments.gmane.org/gmane.comp.freedesktop.dbus/15286
  g_object_unref(session_connection);
  g_object_unref(session_connection);
#endif
}
