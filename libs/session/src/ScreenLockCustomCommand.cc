#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ScreenLockCustomCommand.hh"
#include "debug.hh"

#if defined(HAVE_GLIB)
#  include <glib.h>
#else
#  include <unistd.h>
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

#if defined(HAVE_GLIB)
  GError *error = nullptr;
  if (!g_spawn_command_line_async(cmd_.c_str(), &error))
    {
      g_error_free(error);
      return false;
    }
  return true;
#else
  // POSIX fallback: run the command via /bin/sh in a detached child process
  pid_t pid = fork();
  if (pid == 0)
    {
      execl("/bin/sh", "sh", "-c", cmd_.c_str(), nullptr);
      _exit(1);
    }
  return pid > 0;
#endif
}
