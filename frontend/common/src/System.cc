// Display.cc
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GLIB
#include <glib.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "System.hh"
#include "debug.hh"

#if defined(HAVE_X)
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

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

#if defined(HAVE_X)

gchar *System::xlock = NULL;
bool System::kde = false;

#elif defined(WIN32)

HINSTANCE System::user32_dll = NULL;
System::LockWorkStationFunc System::lock_func = NULL;
bool System::shutdown_supported;

#endif

bool
System::is_lockable()
{
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
  TRACE_ENTER("System::lock");
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
  TRACE_EXIT();
}

bool
System::is_shutdown_supported()
{
  bool ret;
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
          if( HARPOON_ENABLED )
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
#if defined(HAVE_X)
             const char *display
#endif
             )
{
  TRACE_ENTER("System::init");
#if defined(HAVE_X)
  init_kde(display);
  gchar *lock = NULL;
  if (is_kde() && (lock = g_find_program_in_path("kdesktop_lock")))
    {
      xlock = g_strdup_printf("%s --display \"%s\" --forcelock",
                              lock, display);
    }
  else if ((lock = g_find_program_in_path("xscreensaver-command")))
    {
      xlock = g_strdup_printf("%s --display \"%s\" -lock",
                              lock, display);
    }
  else if ((lock = g_find_program_in_path("xlock")))
    {
      xlock = g_strdup_printf("%s -display \"%s\"",
                              lock, display);
    }
  else if ((lock = g_find_program_in_path("gnome-screensaver-command")))
    {
      xlock = g_strdup_printf("%s --lock", lock);
    }
  g_free(lock);
  
  if (xlock != NULL)
    {
      TRACE_MSG("Locking enabled: " << xlock);
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
  TRACE_EXIT();
}


#if defined(HAVE_X)
static bool
get_self_typed_prop (Display *display,
                     Window      xwindow,
                     Atom        atom,
                     unsigned long     *val)
{  
  Atom type;
  int format;
  unsigned long nitems;
  unsigned long bytes_after;
  unsigned long *num;
  
  type = None;
  XGetWindowProperty (display,
                      xwindow,
                      atom,
                      0, 100000,
                      False, atom, &type, &format, &nitems,
                      &bytes_after, (unsigned char **)&num);  

  if (type != atom) {
    return false;
  }

  if (val)
    *val = *num;
  
  XFree (num);

  return true;
}

static bool
has_wm_state (Display *display, Window xwindow)
{
  return get_self_typed_prop (display, xwindow,
                              XInternAtom (display, "WM_STATE", False),
                              NULL);
}

static bool
look_for_kdesktop_recursive (Display *display, Window xwindow)
{
  Window ignored1, ignored2;
  Window *children = NULL;
  unsigned int n_children = 0;
  unsigned int i = 0;
  bool retval;
  
  /* If WM_STATE is set, this is a managed client, so look
   * for the class hint and end recursion. Otherwise,
   * this is probably just a WM frame, so keep recursing.
   */
  if (has_wm_state (display, xwindow)) {      
    XClassHint ch;
      
    ch.res_name = NULL;
    ch.res_class = NULL;
      
    XGetClassHint (display, xwindow, &ch);
      
    if (ch.res_name)
      XFree (ch.res_name);
      
    if (ch.res_class) {
      if (strcasecmp (ch.res_class, "kdesktop") == 0) {
        XFree (ch.res_class);
        return true;
      }
      else
        XFree (ch.res_class);
    }
    return false;
  }
  retval = false;
  
  Status status = XQueryTree(display,
                             xwindow,
                             &ignored1, &ignored2, &children, &n_children);
  if (status)
    {
      i = 0;
      while (i < n_children) {
        if (look_for_kdesktop_recursive (display, children[i])) {
          retval = true;
          break;
        }
        
        ++i;
      }

      if (children)
        XFree (children);
    }

  return retval;
}



void
System::init_kde(const char *display)
{
  TRACE_ENTER("System::init_kde");
  Display * dis = XOpenDisplay(display);
  if (dis != None)
    {
      kde = look_for_kdesktop_recursive (dis, XRootWindow(dis, 0));
      XCloseDisplay(dis);
    }
  TRACE_RETURN(kde);
}
#endif
