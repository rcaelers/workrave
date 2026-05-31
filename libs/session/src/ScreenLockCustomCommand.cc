#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ScreenLockCustomCommand.hh"
#include "debug.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

ScreenLockCustomCommand::ScreenLockCustomCommand(std::string cmd)
  : cmd_(std::move(cmd))
{
}

bool
ScreenLockCustomCommand::lock()
{
  TRACE_ENTRY_PAR(cmd_);
  if (cmd_.empty())
    {
      return false;
    }
  GError *error = nullptr;
  if (!g_spawn_command_line_async(cmd_.c_str(), &error))
    {
      g_error_free(error);
      return false;
    }
  return true;
}
