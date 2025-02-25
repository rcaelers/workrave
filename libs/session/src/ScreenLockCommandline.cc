// SystemLockCommandline.cc -- support for locking the system using command line
//
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
// All rights reserved.
// Uses some code and ideas from the KShutdown utility: file src/actions/lock.cpp
// Copyright (C) 2009  Konrad Twardowski
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

#include "ScreenLockCommandline.hh"
#include "debug.hh"

ScreenLockCommandline::ScreenLockCommandline(const char *program_name, const char *parameters, bool async)
  : async(async)
{
  TRACE_ENTRY_PAR(program_name);
  char *program_path = g_find_program_in_path(program_name);

  if (program_path == nullptr)
    {
      cmd = nullptr;
    }
  else if (parameters != nullptr)
    {
      cmd = g_strdup_printf("%s %s", program_path, parameters);
      g_free(program_path);
    }
  else
    {
      cmd = program_path;
    }
}

bool
ScreenLockCommandline::invoke(const gchar *command, bool async)
{
  GError *error = nullptr;

  if (!async)
    {
      // synchronised call
      gint exit_code;
      if (!g_spawn_command_line_sync(command, nullptr, nullptr, &exit_code, &error))
        {
          g_error_free(error);
          return false;
        }
#if GLIB_CHECK_VERSION(2, 70, 0)
      return g_spawn_check_wait_status(exit_code, nullptr);
#else
      return g_spawn_check_exit_status(exit_code, nullptr);
#endif
    }

  // asynchronous call
  if (!g_spawn_command_line_async(command, &error))
    {
      g_error_free(error);
      return false;
    }
  return true;
}

bool
ScreenLockCommandline::lock()
{
  TRACE_ENTRY_PAR(cmd);
  return invoke(cmd, async);
}
