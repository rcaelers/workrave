// System.hh
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2011 Rob Caelers & Raymond Penners
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

#if defined(PLATFORM_OS_WIN32)
#include <windows.h>
#endif

#if defined(PLATFORM_OS_UNIX)
#include <string>
#endif

#if defined(HAVE_DBUS_GIO)
#include <gio/gio.h>
#endif

class System
{
public:
  static bool is_lockable();
  static bool is_shutdown_supported();
  static void lock();
  static void shutdown();

  static void init(
#if defined(PLATFORM_OS_UNIX)
                   const char *display
#endif
                   );

private:
#ifdef HAVE_DBUS_GIO
  static GDBusProxy *lock_proxy;
#endif

#if defined(PLATFORM_OS_UNIX)
  static void init_kde_lock();
  static bool kde_lock();

  static bool lockable;
  static std::string lock_display;
  static bool shutdown_supported;

#elif defined(PLATFORM_OS_WIN32)
  static bool shutdown_helper(bool for_real);

  typedef HRESULT (FAR PASCAL *LockWorkStationFunc)(void);
  static LockWorkStationFunc lock_func;
  static HINSTANCE user32_dll;
  static bool shutdown_supported;
#endif
};

#endif // SYSTEM_HH
