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


gchar *Display::xlock = NULL;
bool Display::initialized = false;


bool
Display::is_lockable()
{
  init();
  bool ret;
#ifdef HAVE_X
  ret = xlock != NULL;
#else  
  ret = false;
#endif
  return ret;
}

void
Display::lock()
{
  init();
#ifdef HAVE_X
  if (xlock != NULL)
    {
      GString *cmd = g_string_new(xlock);
      cmd = g_string_append_c(cmd, '&');
      system(cmd->str);
      g_string_free(cmd, true);
    }
#endif  
}
  
void
Display::init()
{
  TRACE_ENTER("Display::init");
  if (! initialized)
    {
#ifdef HAVE_X
      xlock = g_find_program_in_path("xlock");
      if (xlock != NULL)
        {
          TRACE_MSG("Locking enabled");
        }
      else
        {
          TRACE_MSG("Locking disabled");
        }
#endif
      initialized = true;
    }
  TRACE_EXIT();
}



