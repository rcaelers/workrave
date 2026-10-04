// Copyright (C) 2013 Rob Caelers & Raymond Penners
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

#include "utils/Platform.hh"
#include <array>

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#endif

#include <stdlib.h>

using namespace std;
using namespace workrave::utils;

std::optional<std::string>
Platform::registry_get_value(const char *path, const char *name)
{
  HKEY handle = nullptr;
  LONG err = 0;

  err = RegOpenKeyExA(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &handle);
  if (err != ERROR_SUCCESS)
    {
      return {};
    }

  DWORD type = 0;
  DWORD size = 0;

  err = RegQueryValueExA(handle, name, 0, &type, nullptr, &size);
  if (err != ERROR_SUCCESS || type != REG_SZ)
    {
      RegCloseKey(handle);
      return {};
    }

  std::string result(size - 1, '\0'); // -1 to account for null terminator
  err = RegQueryValueExA(handle, name, 0, &type, reinterpret_cast<LPBYTE>(result.data()), &size);
  RegCloseKey(handle);

  if (err != ERROR_SUCCESS)
    {
      return {};
    }

  return result;
}

bool
Platform::registry_set_value(const char *path, const char *name, const char *value)
{
  HKEY handle = nullptr;
  bool rc = false;
  DWORD disp = 0;
  LONG err = 0;

  err = RegCreateKeyExA(HKEY_CURRENT_USER, path, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      if (value != nullptr)
        {
          err = RegSetValueExA(handle,
                               name,
                               0,
                               REG_SZ,
                               reinterpret_cast<const BYTE *>(value),
                               static_cast<DWORD>(strlen(value) + 1));
        }
      else
        {
          err = RegDeleteValueA(handle, name);
        }
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  return rc;
}

std::string
Platform::get_application_name()
{
  char app_dir_name[MAX_PATH];
  GetModuleFileNameA(GetModuleHandle(nullptr), app_dir_name, sizeof(app_dir_name));
  // app_dir_name == c:\program files\workrave\lib\workrave.exe
  char *s = strrchr(app_dir_name, '\\');
  return string(s);
}

namespace
{
  constexpr const char *run_key = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

  std::string autostart_command()
  {
    std::array<char, 32768> executable{};
    auto length = GetModuleFileNameA(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
    if (length == 0 || length >= executable.size())
      {
        return {};
      }
    return "\"" + std::string(executable.data(), length) + "\"";
  }
}

bool
Platform::autostart_enabled()
{
  auto command = autostart_command();
  auto value = registry_get_value(run_key, "Workrave");
  return !command.empty() && value && _stricmp(value->c_str(), command.c_str()) == 0;
}

bool
Platform::set_autostart_enabled(bool enabled)
{
  auto command = autostart_command();
  if (command.empty())
    {
      return false;
    }
  if (enabled)
    {
      return registry_set_value(run_key, "Workrave", command.c_str());
    }
  // Disabling Classic must not remove a startup entry owned by Qt, or vice versa.
  return !autostart_enabled() || registry_set_value(run_key, "Workrave", nullptr);
}

std::wstring
Platform::convert(const char *c)
{
  std::string s(c);

  return {s.begin(), s.end()};
}

int
Platform::setenv(const char *name, const char *val, int)
{
  return _putenv_s(name, val);
}

int
Platform::unsetenv(const char *name)
{
  return _putenv_s(name, "");
}

bool
Platform::can_position_windows()
{
  return true;
}

bool
Platform::running_on_wayland()
{
  return false;
}

bool
Platform::is_arm64()
{
#if defined(_M_ARM64) || defined(__aarch64__)
  // Native ARM64 build
  return true;
#elif defined(_M_X64) || defined(__x86_64__)
  // AMD64 build - check if running on ARM64 through emulation
  using LPFN_ISWOW64PROCESS2 = BOOL(WINAPI *)(HANDLE, PUSHORT, PUSHORT);
  HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
  if (kernel32 != nullptr)
    {
      auto proc = GetProcAddress(kernel32, "IsWow64Process2");
      if (proc != nullptr)
        {
          auto fnIsWow64Process2 = LPFN_ISWOW64PROCESS2(proc);
          USHORT processMachine = 0;
          USHORT nativeMachine = 0;

          if (fnIsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine) != FALSE)
            {
              // IMAGE_FILE_MACHINE_ARM64 = 0xAA64
              return nativeMachine == 0xAA64;
            }
        }
    }

  // Fallback: try to detect ARM64 through processor architecture
  SYSTEM_INFO si;
  GetNativeSystemInfo(&si);

  // PROCESSOR_ARCHITECTURE_ARM64 = 12
  return si.wProcessorArchitecture == 12;
#else
  // Other architectures (x86, etc.)
  return false;
#endif
}
