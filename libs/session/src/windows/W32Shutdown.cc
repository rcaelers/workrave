// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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
#  include "config.h"
#endif

#include "W32Shutdown.hh"

#include <spdlog/spdlog.h>
#include <windows.h>
#include <powrprof.h>

namespace
{
  // All three power operations require SeShutdownPrivilege. Restore its previous
  // state afterwards, including when probing support without performing an action.
  class ShutdownPrivilege
  {
  public:
    ShutdownPrivilege()
    {
      if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        {
          spdlog::warn("Cannot open Windows process token for power operations: error {}", GetLastError());
          return;
        }

      TOKEN_PRIVILEGES requested{};
      requested.PrivilegeCount = 1;
      requested.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      if (!LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &requested.Privileges[0].Luid))
        {
          spdlog::warn("Cannot look up Windows shutdown privilege: error {}", GetLastError());
          return;
        }

      DWORD previous_size = sizeof(previous);
      SetLastError(ERROR_SUCCESS);
      bool adjusted = AdjustTokenPrivileges(token, FALSE, &requested, sizeof(previous), &previous, &previous_size) != FALSE;
      DWORD error = GetLastError();
      // AdjustTokenPrivileges may succeed even when the token lacks the privilege.
      enabled = adjusted && error == ERROR_SUCCESS;
      if (!enabled)
        {
          spdlog::warn("Cannot enable Windows shutdown privilege: error {}", error);
        }
    }

    ~ShutdownPrivilege()
    {
      if (enabled && !AdjustTokenPrivileges(token, FALSE, &previous, 0, nullptr, nullptr))
        {
          spdlog::warn("Cannot restore Windows shutdown privilege: error {}", GetLastError());
        }
      if (token != nullptr)
        {
          CloseHandle(token);
        }
    }

    ShutdownPrivilege(const ShutdownPrivilege &) = delete;
    ShutdownPrivilege &operator=(const ShutdownPrivilege &) = delete;

    bool is_enabled() const
    {
      return enabled;
    }

  private:
    HANDLE token{nullptr};
    TOKEN_PRIVILEGES previous{};
    bool enabled{false};
  };
} // namespace

W32Shutdown::W32Shutdown()
{
  ShutdownPrivilege privilege;
  shutdown_supported = privilege.is_enabled();
}

bool
W32Shutdown::canSuspend()
{
  SYSTEM_POWER_CAPABILITIES capabilities{};
  if (!shutdown_supported)
    {
      return false;
    }
  if (!GetPwrCapabilities(&capabilities))
    {
      spdlog::warn("Cannot query Windows sleep support: error {}", GetLastError());
      return false;
    }

  if (capabilities.SystemS1 || capabilities.SystemS2 || capabilities.SystemS3)
    {
      return true;
    }

  // Modern Standby uses S0 low power idle. Query it separately because MinGW's
  // SYSTEM_POWER_CAPABILITIES declaration does not expose the AoAc member.
  POWER_PLATFORM_INFORMATION platform{};
  auto status = CallNtPowerInformation(PlatformInformation, nullptr, 0, &platform, sizeof(platform));
  if (status != 0)
    {
      spdlog::warn("Cannot query Windows Modern Standby support: status {}", status);
      return false;
    }
  return platform.AoAc;
}

bool
W32Shutdown::canHibernate()
{
  SYSTEM_POWER_CAPABILITIES capabilities{};
  if (!shutdown_supported)
    {
      return false;
    }
  if (!GetPwrCapabilities(&capabilities))
    {
      spdlog::warn("Cannot query Windows hibernation support: error {}", GetLastError());
      return false;
    }

  return capabilities.SystemS4 && capabilities.HiberFilePresent;
}

bool
W32Shutdown::shutdown()
{
  ShutdownPrivilege privilege;
  if (!privilege.is_enabled())
    {
      return false;
    }

  // Allow Windows and other applications to cancel shutdown for unsaved work.
  if (!ExitWindowsEx(EWX_POWEROFF, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED))
    {
      spdlog::error("Cannot shut down Windows: error {}", GetLastError());
      return false;
    }
  return true;
}

bool
W32Shutdown::suspend_helper(bool hibernate)
{
  // Recheck availability: hibernation or sleep support can change after startup.
  if (hibernate ? !canHibernate() : !canSuspend())
    {
      return false;
    }

  ShutdownPrivilege privilege;
  if (!privilege.is_enabled())
    {
      return false;
    }
  if (!SetSuspendState(hibernate ? TRUE : FALSE, FALSE, FALSE))
    {
      spdlog::error("Cannot {} Windows: error {}", hibernate ? "hibernate" : "put to sleep", GetLastError());
      return false;
    }
  return true;
}

bool
W32Shutdown::suspend()
{
  return suspend_helper(false);
}

bool
W32Shutdown::hibernate()
{
  return suspend_helper(true);
}
