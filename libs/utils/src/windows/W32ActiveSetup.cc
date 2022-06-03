// W32ActiveSetup.cc --- Active Setup for Workrave
//
// Copyright (C) 2012 Ray Satiro
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

/* Active Setup is post install data that's set by the Inno Setup installer.

After Active Setup data is set it will be run for each user the next time that user logs on.
However, in the case that the user hasn't yet logged off and then back on, any programs specified
by Active Setup have not yet ran.

In order to ensure the proper behavior of Workrave, Workrave's Active Setup key(s) should always be
checked before Workrave loads, and run if they haven't already. This module emulates the behavior
of Microsoft's Active Setup.

##############################################WARNING##############################################
Active Setup is undocumented and can be dangerous if not used properly. Its behavior is blocking
and there is no interaction available to the user in a typical case. In a typical case it will
block the explorer shell, with explorer running the Active Setup programs hidden from view and
waiting for them to complete before completing user initialization on logon. This emulation is very
similar. It was designed for the Workrave application to block before continuing execution until
Workrave's Active Setup programs have run and terminated. Therefore, it is extremely important that
any program put under the purview of Active Setup does not require any user interaction, does not
have any error message boxes, and has all its dependencies (so there are no message boxes about
failure to load libraries). The Workrave Active Setup program (I've only written one so far -- to
change a user's autorun settings) takes an extra step, to void user interaction during any
potential crash.
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"

#include <tchar.h>
#include <cstdlib>
#include <sstream>
#include <iterator>
#include <string>

#include "utils/W32ActiveSetup.hh"

using namespace std;

/* using 32-bit registry view on 32-bit OS, 64-bit registry view on 64-bit OS
http://support.microsoft.com/kb/907660
*/
const REGSAM W32ActiveSetup::registry_view = (W32ActiveSetup::is_os_64() ? KEY_WOW64_64KEY : KEY_WOW64_32KEY);

/* The subkey path to HKLM/HKCU Active Setup GUIDs */
const wchar_t W32ActiveSetup::component_path[] = L"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\";

/* Right now there is only one GUID key set by Inno and it's used by Workrave's 'ChangeAutorun'
program, which adds/updates each user's Workrave autorun setting.
*/
const wchar_t W32ActiveSetup::guid_autorun[] = L"{180B0AC5-6FDA-438B-9466-C9894322B6BA}";

/* W32ActiveSetup::is_os_64()

returns true if the windows operating system is 64-bit
*/
bool
W32ActiveSetup::is_os_64()
{
  SYSTEM_INFO si = {};

  GetNativeSystemInfo(&si);

  return ((si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
          || (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64));
}

/* W32ActiveSetup::get_user_profile_dir()

returns the path contained in environment variable %USERPROFILE%
*/
const WCHAR *
W32ActiveSetup::get_user_profile_dir()
{
  static vector<WCHAR> buffer;

  if (buffer.size() && *buffer.begin())
    return &buffer[0];

  LPCWSTR env_var = L"USERPROFILE";

  DWORD ret = GetEnvironmentVariableW(env_var, NULL, 0);
  if (!ret || (ret > 32767))
    return NULL;

  buffer.resize(ret);

  ret = GetEnvironmentVariableW(env_var, &buffer[0], static_cast<DWORD>(buffer.size()));
  if (!ret || (ret >= buffer.size()) || !*buffer.begin())
    return NULL;

  return &buffer[0];
}

/* W32ActiveSetup::check_guid()

Check for the existence of an Active Setup GUID key (HKLM or HKCU).

returns true if the key exists
*/
bool
W32ActiveSetup::check_guid(const enum reg reg, // HKLM or HKCU
                           const wstring &guid)
{
  HKEY root_hkey = NULL;

  switch (reg)
    {
    case HKLM:
      root_hkey = HKEY_LOCAL_MACHINE;
      break;
    case HKCU:
      root_hkey = HKEY_CURRENT_USER;
      break;
    default:
      return false;
    }

  wstring keyname(W32ActiveSetup::component_path);
  keyname += guid;

  HKEY hkey = NULL;
  LONG ret = RegOpenKeyExW(root_hkey,
                           keyname.c_str(),
                           0,
                           (STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | W32ActiveSetup::registry_view),
                           &hkey);

  if (ret)
    return false;

  CloseHandle(hkey);
  return true;
}

/* W32ActiveSetup::is_guid_enabled()

returns true if the HKLM Active Setup GUID key exists and the key value "IsInstalled" either does
not exist or does exist and does not have both a REG_DWORD type and data 0.

Those conditions are based on the my testing in Windows XP SP3 x86. In most cases for GUID keys
"IsInstalled" exists with a REG_DWORD type and data 1, in which case this function returns true.
*/
bool
W32ActiveSetup::is_guid_enabled(const wstring &guid)
{
  if (guid.empty())
    return false;

  wstring keyname(W32ActiveSetup::component_path);
  keyname += guid;

  HKEY hkey = NULL;
  LONG ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           keyname.c_str(),
                           0,
                           (STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | W32ActiveSetup::registry_view),
                           &hkey);

  if (ret)
    return false;

  DWORD type = REG_NONE;
  DWORD data = 0;
  DWORD bytelen = sizeof(data);
  ret = RegQueryValueExW(hkey, L"IsInstalled", NULL, &type, (LPBYTE)&data, &bytelen);

  CloseHandle(hkey);

  if (!ret && (type == REG_DWORD) && (bytelen == sizeof(data)) && !data)
    return false;

  return true;
}

/* W32ActiveSetup::read_from_registry_value()

Get the REG_SZ data from a registry value in an Active Setup GUID key (HKLM or HKCU).

Before this function does anything else 'data' is cleared. 'data' may be empty even on success.

returns true if the value exists in the registry. 'data' receives the value's data, if any.
*/
bool
W32ActiveSetup::read_from_registry_value(const enum reg reg, // HKLM or HKCU
                                         const wstring &guid,
                                         const wstring &value,
                                         wstring &data // out
)
{
  data.clear();

  if (guid.empty() || value.empty())
    return false;

  HKEY root_hkey = NULL;

  switch (reg)
    {
    case HKLM:
      root_hkey = HKEY_LOCAL_MACHINE;
      break;
    case HKCU:
      root_hkey = HKEY_CURRENT_USER;
      break;
    default:
      return false;
    }

  wstring keyname(W32ActiveSetup::component_path);
  keyname += guid;

  HKEY hkey = NULL;
  LONG ret = RegOpenKeyExW(root_hkey,
                           keyname.c_str(),
                           0,
                           (STANDARD_RIGHTS_READ | KEY_QUERY_VALUE | W32ActiveSetup::registry_view),
                           &hkey);

  if (ret)
    return false;

  DWORD type = REG_NONE, bytelen = 0;
  ret = RegQueryValueExW(hkey, value.c_str(), NULL, &type, NULL, &bytelen);

  if (ret || !bytelen || (type != REG_SZ))
    {
      CloseHandle(hkey);
      return false;
    }

  vector<BYTE> buffer(bytelen + sizeof(wchar_t));

  ret = RegQueryValueExW(hkey, value.c_str(), NULL, &type, &buffer[0], &bytelen);

  CloseHandle(hkey);

  if (ret || (type != REG_SZ))
    return false;

  data = (wchar_t *)&buffer[0];
  return true;
}

/* W32ActiveSetup::write_to_registry_value()

Set a value and its REG_SZ data in an Active Setup GUID key (HKCU only).

If the HKCU Active Setup GUID key does not exist this function will create it.
If 'value' is empty this function sets the GUID key's default value.
'data' can also be an empty string.

returns true if the value and its data (if any) were set in the registry.
*/
bool
W32ActiveSetup::write_to_registry_value(const wstring &guid, const wstring &value, const wstring &data)
{
  if (guid.empty())
    return false;

  wstring keyname(W32ActiveSetup::component_path);
  keyname += guid;

  HKEY hkey = NULL;

  LONG ret = RegCreateKeyExW(HKEY_CURRENT_USER,
                             keyname.c_str(),
                             0,
                             NULL,
                             0,
                             (KEY_READ | KEY_WRITE | W32ActiveSetup::registry_view),
                             NULL,
                             &hkey,
                             NULL);

  if (ret)
    return false;

  ret = RegSetValueExW(hkey,
                       value.c_str(),
                       0,
                       REG_SZ,
                       (const BYTE *)data.c_str(),
                       static_cast<int>((data.length() + 1) * sizeof(wchar_t)));

  CloseHandle(hkey);

  return !ret;
}

/* W32ActiveSetup::get_version()
Converts the REG_SZ data from value "Version" in an Active Setup GUID key (HKLM or HKCU) into a
vector of 4 DWORDs.

Active Setup observations:
A valid "Version" value string of a GUID key has four parts or less, separated by comma. If a
number is greater than dword max the version is still valid, and that number is wrapped around.

"2012,05,10,023701" returns DWORDs 2012, 5, 10, 23701
"1" returns DWORDs 1, 0, 0, 0
"" or "0" returns DWORDs 0, 0, 0, 0
"1,2,3,4,5" (invalid) returns DWORDs 0, 0, 0, 0
"25,4294967299,77,4" returns DWORDs 25, 3, 77, 4

Before this function does anything else 'version' is cleared and then resized to 4 DWORDs.

returns true if the "Version" value exists in the registry and it's valid. 'version' receives the
version as a vector of 4 DWORDs.
*/
bool
W32ActiveSetup::get_version(const enum reg reg, // HKLM or HKCU
                            const wstring &guid,
                            vector<DWORD> &version // out
)
{
  version.clear();
  version.resize(4, 0);

  wstring data;
  if (!read_from_registry_value(reg, guid, L"Version", data))
    return false;

  /* testing shows anything other than these characters invalidates the version string.
  Active Setup treats it the same as a version where all parts are zeroes
  */
  size_t pos = data.find_first_not_of(L"0123456789,");
  if (pos != wstring::npos)
    return false;

  /* testing shows if there is more than 3 commas that invalidates the version string.
  Active Setup treats it the same as a version where all parts are zeroes
  */
  for (unsigned i = 0, count = 0; i < data.length(); ++i)
    {
      if (data[i] == L',')
        {
          ++count;
          if (count > 3)
            return false;
        }
    }

  wstringstream ss(data);

  for (unsigned i = 0; i < 4; ++i)
    {
      wstring s;

      if (!getline(ss, s, L','))
        break;

      for (unsigned j = 0; j < s.length(); ++j)
        {
          version[i] *= 10;
          version[i] += s[j] - 48;
        }
    }

  return true;
}

/* W32ActiveSetup::set_version()

Converts a vector of 4 DWORDs to a comma separated string of unsigned numbers and sets it as the
REG_SZ data for the "Version" value of an Active Setup GUID key.

returns true on success
*/
bool
W32ActiveSetup::set_version(const wstring &guid, const vector<DWORD> &version)
{
  if (version.size() != 4)
    return false;

  wstringstream ss;
  ss << version[0] << "," << version[1] << "," << version[2] << "," << version[3];

  return write_to_registry_value(guid, L"Version", ss.str());
}

/* W32ActiveSetup::update()

This function emulates Active Setup behavior. It should be called synchronously by the main thread
before Workrave is initialized.

The way Active Setup basically works is it checks the HKLM GUID key for a "Version" REG_SZ value
and compares it to the same HKCU GUID key's "Version" value. If the HKCU GUID key does not exist,
or its "Version" value does not exist, or its version is less than the HKLM version then Active
Setup will add/update the HKCU GUID key by copying any HKLM "Version" value to the HKCU "Version"
value, and then execute the string contained in HKLM GUID key's "StubPath" value, if any.
http://www.sepago.de/helge/2010/04/22/active-setup-explained/

returns true if the HKLM Active Setup GUID key needed to be added/updated in HKCU Active Setup.
*/
bool
W32ActiveSetup::update(const wstring &guid)
{
  if (guid.empty() || !check_guid(HKLM, guid) || !is_guid_enabled(guid))
    return false;

  vector<DWORD> hklm_version;
  get_version(HKLM, guid, hklm_version);

  if (check_guid(HKCU, guid))
    {
      vector<DWORD> hkcu_version;
      get_version(HKCU, guid, hkcu_version);

      if (hkcu_version >= hklm_version)
        return false;
    }

  if (!set_version(guid, hklm_version))
    return false;

  // At this point HKCU Active Setup has been updated so any return should be true

  wstring data;

  if (read_from_registry_value(HKLM, guid, L"Locale", data))
    write_to_registry_value(guid, L"Locale", data);

  if (!read_from_registry_value(HKLM, guid, L"StubPath", data) || data.empty())
    return true;

  vector<WCHAR> buffer(data.begin(), data.end());
  buffer.push_back(L'\0');

  HANDLE thread = CreateThread(NULL, 0, create_process, &buffer[0], 0, NULL);
  if (thread)
    {
      WaitForSingleObject(thread, INFINITE);
      CloseHandle(thread);
      thread = NULL;
    }

  /* Active Setup only attempts to run the StubPath, it doesn't record whether or not it was
  successful in doing so.
  */

  return true;
}

/* W32ActiveSetup::update_all()

returns true if any of Workrave's HKLM Active Setup GUID keys needed to be installed in HKCU
Active Setup.
*/
bool
W32ActiveSetup::update_all()
{
  /* For now there's only one entry, for setting up Workrave's autorun for each user */
  return update(W32ActiveSetup::guid_autorun);
}

/* W32ActiveSetup::create_process()

This is the starting function for a thread that creates a process and waits for it to terminate.
*/
DWORD WINAPI
W32ActiveSetup::create_process(LPVOID lpParam)
{
  DWORD exit_code = ((DWORD)-1);
  WCHAR *command = (WCHAR *)lpParam;
  STARTUPINFOW si = {
    sizeof(si),
  };
  PROCESS_INFORMATION pi = {
    INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE,
  };

  if (!command || !*command)
    return exit_code;

  void *OldValue = NULL;
  bool wow64_disabled = false;
  BOOL(WINAPI * pWow64DisableWow64FsRedirection)(void **) = NULL;
  BOOL(WINAPI * pWow64RevertWow64FsRedirection)(void *) = NULL;

  if (W32ActiveSetup::is_os_64())
    {
      pWow64DisableWow64FsRedirection = (BOOL(WINAPI *)(void **))GetProcAddress(GetModuleHandleA("kernel32"),
                                                                                "Wow64DisableWow64FsRedirection");

      pWow64RevertWow64FsRedirection = (BOOL(WINAPI *)(void *))GetProcAddress(GetModuleHandleA("kernel32"),
                                                                              "Wow64RevertWow64FsRedirection");
    }

  if (pWow64DisableWow64FsRedirection && pWow64RevertWow64FsRedirection)
    wow64_disabled = !!pWow64DisableWow64FsRedirection(&OldValue);

  BOOL ret = CreateProcessW(NULL, command, NULL, NULL, FALSE, 0, NULL, get_user_profile_dir(), &si, &pi);

  if (wow64_disabled && pWow64DisableWow64FsRedirection && pWow64RevertWow64FsRedirection)
    wow64_disabled = !pWow64RevertWow64FsRedirection(OldValue);

  if (ret)
    {
      if (pi.hProcess != INVALID_HANDLE_VALUE)
        {
          WaitForSingleObject(pi.hProcess, INFINITE);

          if (!GetExitCodeProcess(pi.hProcess, &exit_code))
            exit_code = ((DWORD)-1);

          CloseHandle(pi.hProcess);
        }

      if (pi.hThread != INVALID_HANDLE_VALUE)
        CloseHandle(pi.hThread);
    }

  return exit_code;
}
