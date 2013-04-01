// System.cc
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "System.hh"
#include "debug.hh"

#if defined(PLATFORM_OS_UNIX)
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef HAVE_APP_GTK
#include <gdk/gdkx.h>
#endif
#endif

#if defined(HAVE_UNIX)
#include <sys/wait.h>
#endif

#ifdef PLATFORM_OS_WIN32
#include <shlobj.h>
#include <shldisp.h>
#include "harpoon.h"

#include "CoreFactory.hh"
#include "config/IConfigurator.hh"

using namespace workrave;

#ifndef HAVE_ISHELLDISPATCH
#undef INTERFACE
#define INTERFACE IShellDispatch
DECLARE_INTERFACE_(IShellDispatch, IUnknown)
{
  STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS) PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;
  STDMETHOD_(ULONG,dummy1)(THIS) PURE;
  STDMETHOD_(ULONG,dummy2)(THIS) PURE;
  STDMETHOD_(ULONG,dummy3)(THIS) PURE;
  STDMETHOD_(ULONG,dummy4)(THIS) PURE;
  STDMETHOD_(ULONG,dummy5)(THIS) PURE;
  STDMETHOD_(ULONG,dummy6)(THIS) PURE;
  STDMETHOD_(ULONG,dummy7)(THIS) PURE;
  STDMETHOD_(ULONG,dummy8)(THIS) PURE;
  STDMETHOD_(ULONG,dummy9)(THIS) PURE;
  STDMETHOD_(ULONG,dummya)(THIS) PURE;
  STDMETHOD_(ULONG,dummyb)(THIS) PURE;
  STDMETHOD_(ULONG,dummyc)(THIS) PURE;
  STDMETHOD_(ULONG,dummyd)(THIS) PURE;
  STDMETHOD_(ULONG,dummye)(THIS) PURE;
  STDMETHOD_(ULONG,dummyf)(THIS) PURE;
  STDMETHOD_(ULONG,dummyg)(THIS) PURE;
  STDMETHOD_(ULONG,dummyh)(THIS) PURE;
        STDMETHOD(ShutdownWindows)(THIS) PURE;
  STDMETHOD_(ULONG,dummyi)(THIS) PURE;
  STDMETHOD_(ULONG,dummyj)(THIS) PURE;
  STDMETHOD_(ULONG,dummyk)(THIS) PURE;
  STDMETHOD_(ULONG,dummyl)(THIS) PURE;
  STDMETHOD_(ULONG,dummym)(THIS) PURE;
  STDMETHOD_(ULONG,dummyn)(THIS) PURE;
  STDMETHOD_(ULONG,dummyo)(THIS) PURE;
  STDMETHOD_(ULONG,dummyp)(THIS) PURE;
  STDMETHOD_(ULONG,dummyq)(THIS) PURE;
END_INTERFACE
};
typedef IShellDispatch *LPSHELLDISPATCH;
#endif

//uuid(D8F015C0-C278-11CE-A49E-444553540000);
const GUID IID_IShellDispatch =
{
  0xD8F015C0, 0xc278, 0x11ce,
  { 0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54 }
};
// 13709620-C279-11CE-A49E-444553540000
const GUID CLSID_Shell =
{
  0x13709620, 0xc279, 0x11ce,
  { 0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54 }
};
#endif /* PLATFORM_OS_WIN32 */

#if defined(PLATFORM_OS_UNIX)

bool System::lockable = false;
std::string System::lock_display;
bool System::shutdown_supported;

#elif defined(PLATFORM_OS_WIN32)

HINSTANCE System::user32_dll = NULL;
System::LockWorkStationFunc System::lock_func = NULL;
bool System::shutdown_supported;

#endif

#ifdef HAVE_DBUS
GDBusProxy *System::lock_proxy =  NULL;
#endif

bool
System::is_lockable()
{
  bool ret;
#if defined(PLATFORM_OS_UNIX)
  ret = lockable;
#elif defined(PLATFORM_OS_WIN32)
  ret = lock_func != NULL;
#else
  ret = false;
#endif
  return ret;
}

#if defined(PLATFORM_OS_UNIX)
invoke(const gchar* command, bool async = false)
{
  GError *error = NULL;

  if(!async)
    {
      // synchronised call
      gint exit_code;
      if (!g_spawn_command_line_sync(command, NULL, NULL, &exit_code, &error) )
        {
          g_error_free(error);
          return false;
        }
      return WEXITSTATUS(exit_code) == 0;
    }
  else
    {
      // asynchronous call
      if (!g_spawn_command_line_async(command, &error) )
        {
          g_error_free(error);
          return false;
        }
      return true;
    }
}
#endif


#if defined(PLATFORM_OS_UNIX) && defined(HAVE_DBUS)
void
System::init_kde_lock()
{
  TRACE_ENTER("System::init_dbus_lock");
	GError *error = NULL;
  lock_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                             G_DBUS_PROXY_FLAGS_NONE,
                                             NULL,
                                             "org.kde.screensaver",
                                             "/ScreenSaver",
                                             "org.freedesktop.ScreenSaver",
                                             NULL,
                                             &error);

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }

  if (lock_proxy != NULL)
    {
      GVariant *result = g_dbus_proxy_call_sync(lock_proxy,
                                                "GetActive",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);
      
      if (result != NULL)
        {
          g_variant_unref(result);
        }
      
      if (error != NULL)
        {
          g_error_free(error);
          g_object_unref(lock_proxy);
          lock_proxy = NULL;
        }
    }

  TRACE_EXIT();
}


bool
System::kde_lock()
{
  bool ret = false;
  
  if (lock_proxy != NULL)
    {
      GError *error = NULL;
      GVariant *result = g_dbus_proxy_call_sync(lock_proxy,
                                                "Lock",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);

      if (result != NULL)
        {
          g_variant_unref(result);
        }

      if (error != NULL)
        {
          g_error_free(error);
          ret = false;
        }
      else
        {
          ret = true;
        }
    }
  
  return ret;
}
#endif

void
System::lock()
{
  TRACE_ENTER("System::lock");
  if (is_lockable())
    {
#if defined(PLATFORM_OS_UNIX)
      gchar *program = NULL, *cmd = NULL;

#ifdef HAVE_DBUS
      if (kde_lock())
        {
          goto end;
        }
#endif
      if ((program = g_find_program_in_path("xscreensaver-command")))
        {
          cmd = g_strdup_printf("%s --display \"%s\" -lock",
                                program, lock_display.c_str() );
          if(invoke(cmd, false) )
            goto end;
        }
      if ((program = g_find_program_in_path("gnome-screensaver-command")))
        {
          cmd = g_strdup_printf("%s --lock", program);
          if(invoke(cmd, false) )
            goto end;
        }
      if ((program = g_find_program_in_path("xlock")))
        {
          cmd = g_strdup_printf("%s -display \"%s\"",
                                program, lock_display.c_str() );
          invoke(cmd, true);
        }
end:  // cleanup of created strings, jump to avoid duplication
      g_free(program);
      g_free(cmd);
#elif defined(PLATFORM_OS_WIN32)
      (*lock_func)();
#endif
    }
  TRACE_EXIT();
}

bool
System::is_shutdown_supported()
{
  bool ret;
#if defined(PLATFORM_OS_UNIX)
  ret = shutdown_supported;
#elif defined(PLATFORM_OS_WIN32)
  ret = shutdown_supported;
#else
  ret = false;
#endif
  return ret;
}

void
System::shutdown()
{
#if defined(PLATFORM_OS_UNIX)
  gchar *program = NULL, *cmd = NULL;

  if ((program = g_find_program_in_path("gnome-session-save")))
    {
      cmd = g_strdup_printf("%s --kill", program);
      invoke(cmd, false);
    }
#elif defined(PLATFORM_OS_WIN32)
  shutdown_helper(true);
#endif
}

#ifdef PLATFORM_OS_WIN32
bool
System::shutdown_helper(bool for_real)
{
  bool ret = false;
  IShellDispatch* pShellDispatch = NULL;
  if (SUCCEEDED(::CoCreateInstance(CLSID_Shell, NULL, CLSCTX_SERVER,
                                   IID_IShellDispatch,
                                   (LPVOID*)&pShellDispatch)))
    {
      ret = true;
      if (for_real)
        {
          harpoon_unblock_input();
          pShellDispatch->ShutdownWindows();
        }
      pShellDispatch->Release();
    }
  return ret;
}
#endif

void
System::init(
#if defined(PLATFORM_OS_UNIX)
             const char *display
#endif
             )
{
  TRACE_ENTER("System::init");
#if defined(PLATFORM_OS_UNIX)
#ifdef HAVE_DBUS
  init_kde_lock();
#endif

  gchar *program;
  if ((program = g_find_program_in_path("xscreensaver-command")) != NULL)
    lockable = true;
  else if ((program = g_find_program_in_path("gnome-screensaver-command")) != NULL)
    lockable = true;
  else if ((program = g_find_program_in_path("xlock")) != NULL)
    lockable = true;

  if (lockable && display != NULL)
    {
      g_free(program);
      lock_display = display;
      TRACE_MSG("Locking enabled");
    }
  else
    {
      TRACE_MSG("Locking disabled");
    }

  shutdown_supported = false;
  if (!shutdown_supported && (g_find_program_in_path("gnome-session-save") != NULL))
    {
      shutdown_supported = true;
    }

#elif defined(PLATFORM_OS_WIN32)
  // Note: this memory is never freed
  user32_dll = LoadLibrary("user32.dll");
  if (user32_dll != NULL)
    {
      lock_func = (LockWorkStationFunc)
        GetProcAddress(user32_dll, "LockWorkStation");
    }
  shutdown_supported = shutdown_helper(false);
#endif
  TRACE_EXIT();
}
