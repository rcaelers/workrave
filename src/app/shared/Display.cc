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
#include <stdio.h>

#include "Display.hh"
#include "debug.hh"


bool Display::initialized = false;

#if defined(HAVE_X)
gchar *Display::xlock = NULL;
#elif defined(WIN32)
HINSTANCE Display::user32_dll = NULL;
Display::LockWorkStationFunc Display::lock_func = NULL;
#endif

bool
Display::is_lockable()
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
Display::lock()
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
  
void
Display::init()
{
  TRACE_ENTER("Display::init");
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
#endif
      initialized = true;
    }
  TRACE_EXIT();
}



