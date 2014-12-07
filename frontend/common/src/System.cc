// System.cc
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <iostream>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "System.hh"
#include "debug.hh"

#if defined(PLATFORM_OS_UNIX)
#include "ScreenLockCommandline.hh"

#ifdef HAVE_DBUS_GIO
#include "ScreenLockDBus.hh"
#include "SystemStateChangeConsolekit.hh"
#include "SystemStateChangeLogind.hh"
#include "SystemStateChangeUPower.hh"
#endif
#endif //PLATFORM_OS_UNIX

#ifdef PLATFORM_OS_WIN32
#include "W32Shutdown.hh"
#include "W32LockScreen.hh"
#endif

#if defined(HAVE_UNIX)
#include <sys/wait.h>
#endif

std::vector<IScreenLockMethod *> System::lock_commands;
std::vector<ISystemStateChangeMethod *> System::system_state_commands;
std::vector<System::SystemOperation> System::supported_system_operations;

#if defined(PLATFORM_OS_UNIX)

#ifdef HAVE_DBUS_GIO
GDBusConnection* System::session_connection = NULL;
GDBusConnection* System::system_connection = NULL;
#endif
#endif


#ifdef PLATFORM_OS_UNIX
#ifdef HAVE_DBUS_GIO
void
System::init_DBus()
{
  TRACE_ENTER("System::init_dbus()");
  //session_connection = workrave::CoreFactory::get_dbus()->get_connection();

  
  GError *error = NULL;
  session_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
  if (error != NULL)
    {
      //it is rare and serious, so report it the user
      std::cerr << "Cannot establish connection to the session bus: " << error->message << std::endl;
      g_error_free(error);
      error = NULL;
    }

  system_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
  if (error != NULL) 
    {
      std::cerr << "Cannot establish connection to the system bus: " << error->message << std::endl;
      g_error_free(error);
      error = NULL;
    }

  TRACE_EXIT();
}


inline bool System::add_DBus_lock_cmd(
    const char *dbus_name, const char *dbus_path, const char *dbus_interface,
    const char *dbus_lock_method, const char *dbus_method_to_check_existence)
{
  TRACE_ENTER_MSG("System::add_DBus_lock_cmd", dbus_name);

  // I wish we could use std::move here
  IScreenLockMethod *lock_method = NULL;
  lock_method = new ScreenLockDBus(session_connection,
          dbus_name, dbus_path, dbus_interface,
          dbus_lock_method, dbus_method_to_check_existence);
  if (!lock_method->is_lock_supported())
    {
      delete lock_method;
      lock_method = NULL;
      TRACE_RETURN(false);
      return false;
    }
  else
    {
      lock_commands.push_back(lock_method);
      TRACE_RETURN(true);
      return true;
    }

}

void
System::init_DBus_lock_commands()
{
  TRACE_ENTER("System::init_DBus_lock_commands");



  if (session_connection)
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
      //WORKS: Ubuntu 12.04: GNOME 3 fallback, Unity
      add_DBus_lock_cmd(
            "org.gnome.ScreenSaver", "/org/gnome/ScreenSaver", "org.gnome.ScreenSaver",
            "Lock", "GetActive");

      //  Cinnamon:   https://github.com/linuxmint/cinnamon-screensaver/blob/master/doc/dbus-interface.html
      //    Same api as GNOME, but with different name,
      add_DBus_lock_cmd(
            "org.cinnamon.ScreenSaver", "/org/cinnamon/ScreenSaver", "org.cinnamon.ScreenSaver",
            "Lock", "GetActive");

      //  Mate: https://github.com/mate-desktop/mate-screensaver/blob/master/doc/dbus-interface.xml
      //  Like GNOME
      add_DBus_lock_cmd(
                  "org.mate.ScreenSaver", "/org/mate/ScreenSaver", "org.mate.ScreenSaver",
                  "Lock", "GetActive");


      //The FreeDesktop API - the most important and most widely supported
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

      //Is only partially implemented by GNOME, so GNOME has to go before
      //Works correctly on KDE4 (Ubuntu 12.04)
      add_DBus_lock_cmd(
            "org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver",
            "Lock", "GetActive");

      //	KDE - old screensaver API - partially verified both
      add_DBus_lock_cmd(
                  "org.kde.screensaver", "/ScreenSaver", "org.freedesktop.ScreenSaver",
                  "Lock", "GetActive");
      add_DBus_lock_cmd(
                  "org.kde.krunner", "/ScreenSaver", "org.freedesktop.ScreenSaver",
                  "Lock", "GetActive");

      //              - there some accounts that when org.freedesktop.ScreenSaver does not work, this works:
      //                      qdbus org.kde.ksmserver /ScreenSaver Lock
      //                      but it is probably a side effect of the fact that implementation of org.kde.ksmserver
      //                      is in the same process as of org.freedesktop.ScreenSaver
      add_DBus_lock_cmd(
                        "org.kde.ksmserver", "/ScreenSaver", "org.freedesktop.ScreenSaver",
                        "Lock", "GetActive");

      // EFL:
      add_DBus_lock_cmd(
                  "org.enlightenment.wm.service", "/org/enlightenment/wm/RemoteObject", "org.enlightenment.wm.Desktop",
                  "Lock", NULL);
    }
  TRACE_EXIT();
}
#endif //HAVE_DBUS_GIO

inline void System::add_cmdline_lock_cmd(
        const char *command_name, const char *parameters, bool async)
{
  TRACE_ENTER_MSG("System::add_cmdline_lock_cmd", command_name);
  IScreenLockMethod *lock_method = NULL;
  lock_method = new ScreenLockCommandline(command_name, parameters, async);
  if (!lock_method->is_lock_supported())
    {
      delete lock_method;
      lock_method = NULL;
      TRACE_RETURN(false);
    }
  else
    {
      lock_commands.push_back(lock_method);
      TRACE_RETURN(true);
    }
}

void System::init_cmdline_lock_commands(const char *display)
{
  TRACE_ENTER_MSG("System::init_cmdline_lock_commands", display);

  //Works: XFCE, i3, LXDE
  add_cmdline_lock_cmd("gnome-screensaver-command", "--lock", false);
  add_cmdline_lock_cmd("mate-screensaver-command", "--lock", false);
  add_cmdline_lock_cmd("enlightenment_remote", "-desktop-lock", false);
  add_cmdline_lock_cmd("xdg-screensaver", "lock", false);

  if (display != NULL)
    {
      char *cmd = g_strdup_printf("-display \"%s\" -lock", display);
      add_cmdline_lock_cmd("xscreensaver-command", cmd, false);
      g_free(cmd);
      cmd = NULL;
    }
  else
    {
      add_cmdline_lock_cmd("xscreensaver-command", "-lock", false);
    }


  //these two may call slock, which may be not user-friendly
  //add_cmdline_lock_cmd("xflock4", NULL, true);
  //add_cmdline_lock_cmd("lxlock", NULL, true);

  if (display != NULL)
    {
      char *cmd = g_strdup_printf("-display \"%s\"", display);
      add_cmdline_lock_cmd("xlock", cmd, true);
      g_free(cmd);
      cmd = NULL;
    }
  else
    {
      add_cmdline_lock_cmd("xlock", NULL, true);
    }

  TRACE_EXIT();
}
#endif

bool
System::lock_screen()
{
  TRACE_ENTER("System::lock");

  for (std::vector<IScreenLockMethod *>::iterator iter = lock_commands.begin();
      iter != lock_commands.end(); ++iter)
    {
      if ((*iter)->lock())
        {
          TRACE_RETURN(true);
          return true;
        }
    }

  TRACE_RETURN(false);
  return false;
}

#if defined(PLATFORM_OS_UNIX) && defined(HAVE_DBUS_GIO)

void System::add_DBus_system_state_command(
      ISystemStateChangeMethod *method)
{
  TRACE_ENTER("System::add_DBus_system_state_command");

  if (method->canDoAnything())
    {
      system_state_commands.push_back(method);
      TRACE_MSG("DBus service is useful");
    }
  else
    {
      delete method;
      method = NULL;
      TRACE_MSG("DBus service is useless");
    }

  TRACE_EXIT();
}

void System::init_DBus_system_state_commands()
{
  TRACE_ENTER("System::init_DBus_system_state_commands");
  if (system_connection)
    {

      //These three DBus interfaces are too diverse
      //to implement support for them in one class
      //Logind is the future so it goes first
      add_DBus_system_state_command(
                          new SystemStateChangeLogind(system_connection));

      add_DBus_system_state_command(
                          new SystemStateChangeUPower(system_connection));

      //ConsoleKit is deprecated so goes last
      add_DBus_system_state_command(
                          new SystemStateChangeConsolekit(system_connection));

      //Other interfaces:
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
#undef ADD_DBUS_SERVICE


    }
  TRACE_EXIT();
}

#endif //PLATFORM_OS_UNIX

bool System::execute(SystemOperation::SystemOperationType type)
{
  TRACE_ENTER("System::execute");
  if (type == SystemOperation::SYSTEM_OPERATION_NONE)
    {
      return false;
    }
  else if (type == SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN)
    {
      return lock_screen();
    }
  else
    {
      for (std::vector<ISystemStateChangeMethod*>::iterator iter = system_state_commands.begin();
                      iter != system_state_commands.end(); ++iter)
        {
          bool ret = false;
          switch (type)
            {
              case SystemOperation::SYSTEM_OPERATION_SHUTDOWN:
                ret = ((*iter)->shutdown());
                break;
              case SystemOperation::SYSTEM_OPERATION_SUSPEND:
                ret = ((*iter)->suspend());
                break;
              case SystemOperation::SYSTEM_OPERATION_HIBERNATE:
                ret = ((*iter)->hibernate());
                break;
              case SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID:
                ret = ((*iter)->suspendHybrid());
                break;
              default:
                throw "System::execute: Unknown system operation";
            };
          if (ret)
            {
              TRACE_RETURN(true);
              return true;
            }
        }
    }
  TRACE_RETURN(false);
  return false;
}

void
System::init(
#if defined(PLATFORM_OS_UNIX)
             const char *display
#endif
             )
{
  TRACE_ENTER("System::init");
#if defined(PLATFORM_OS_UNIX)
#ifdef HAVE_DBUS_GIO
  init_DBus();
  init_DBus_lock_commands();
  init_DBus_system_state_commands();
#endif
  init_cmdline_lock_commands(display);

#elif defined(PLATFORM_OS_WIN32)
  IScreenLockMethod *winLock = new W32LockScreen();
  if (winLock->is_lock_supported())
    {
      lock_commands.push_back(winLock);
    }
  else
    {
      delete winLock;
      winLock = NULL;
    }

  ISystemStateChangeMethod *winShut = new W32Shutdown();
  if (winShut->canShutdown())
    {
      system_state_commands.push_back(winShut);
    }
  else
    {
      delete winShut;
      winShut = NULL;
    }
#endif //defined (PLATFORM_OS_WIN32)

  if (is_lockable())
    {
      supported_system_operations.push_back(
          SystemOperation("Lock", SystemOperation::SYSTEM_OPERATION_LOCK_SCREEN));
    }

  for (std::vector<ISystemStateChangeMethod*>::iterator iter = system_state_commands.begin();
      iter != system_state_commands.end(); ++iter)
    {
      if ((*iter)->canShutdown())
        {
          supported_system_operations.push_back(
              SystemOperation("Shutdown", SystemOperation::SYSTEM_OPERATION_SHUTDOWN));
        }
      if ((*iter)->canSuspend())
        {
          supported_system_operations.push_back(
              SystemOperation("Suspend", SystemOperation::SYSTEM_OPERATION_SUSPEND));
        }
      if ((*iter)->canHibernate())
        {
          supported_system_operations.push_back(
              SystemOperation("Hibernate", SystemOperation::SYSTEM_OPERATION_HIBERNATE));
        }
      if ((*iter)->canSuspendHybrid())
        {
          supported_system_operations.push_back(
              SystemOperation("Suspend hybrid", SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID));
        }
    }

  std::sort(supported_system_operations.begin(), supported_system_operations.end());

  TRACE_EXIT();
}

void
System::clear()
{
  for (std::vector<IScreenLockMethod *>::iterator iter = lock_commands.begin();
        iter != lock_commands.end(); ++iter)
    {
      delete *iter;
    }
  lock_commands.clear();

  for (std::vector<ISystemStateChangeMethod*>::iterator iter = system_state_commands.begin();
      iter != system_state_commands.end(); ++iter)
    {
      delete *iter;
    }
  system_state_commands.clear();
#ifdef HAVE_DBUS_GIO
  //we shouldn't call g_dbus_connection_close_sync here:
  //http://comments.gmane.org/gmane.comp.freedesktop.dbus/15286
  g_object_unref(session_connection);
  g_object_unref(session_connection);
#endif
}
//TODO: lock before suspend/hibernate
