// Display.cc
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#include "config.h"
#include <glib.h>
#include <stdlib.h>

#include "System.hh"
#include "debug.hh"

#ifdef WIN32
#include <shlobj.h>
#include "harpoon.h"

/* MinGW does not have this one yet */
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

#endif /* WIN32 */

bool System::initialized = false;


#if defined(HAVE_X)

gchar *System::xlock = NULL;

#elif defined(WIN32)

HINSTANCE System::user32_dll = NULL;
System::LockWorkStationFunc System::lock_func = NULL;
bool System::shutdown_supported;

#endif

bool
System::is_lockable()
{
  init();
  bool ret;
#if defined(HAVE_X)
  ret = xlock != NULL;
#elif defined(WIN32)
  ret = lock_func != NULL;
#else
  ret = false;
#endif
  return ret;
}

void
System::lock()
{
  if (is_lockable())
    {
#if defined(HAVE_X)
      GString *cmd = g_string_new(xlock);
      cmd = g_string_append_c(cmd, '&');
      system(cmd->str);
      g_string_free(cmd, true);
#elif defined(WIN32)
      (*lock_func)();
#endif  
    }
}

bool
System::is_shutdown_supported()
{
  bool ret;
  init();
#if defined(HAVE_X)
  ret = false;
#elif defined(WIN32)
  ret = shutdown_supported;
#else
  ret = false;
#endif
  return ret;
}

void
System::shutdown()
{
#if defined(HAVE_X)
#elif defined(WIN32)
  shutdown_helper(true);
#endif  
}

#ifdef WIN32
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
System::init()
{
  TRACE_ENTER("System::init");
  if (! initialized)
    {
#if defined(HAVE_X)
      // Note: this memory is never freed
      xlock = g_find_program_in_path("xlock");
      if (xlock != NULL)
        {
          TRACE_MSG("Locking enabled");
        }
      else
        {
          TRACE_MSG("Locking disabled");
        }
#elif defined(WIN32)
      // Note: this memory is never freed
      user32_dll = LoadLibrary("user32.dll");
      if (user32_dll != NULL)
        {
          lock_func = (LockWorkStationFunc)
            GetProcAddress(user32_dll, "LockWorkStation");
        }
      shutdown_supported = shutdown_helper(false);
#endif
      initialized = true;
    }
  TRACE_EXIT();
}



