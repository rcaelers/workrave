// Platform.hh --- Platform class
//
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
#include "config.h"
#endif

#include <assert.h>

#ifdef PLATFORM_OS_WIN32
#include <windows.h>
#endif

#include "utils/Platform.hh"

#include <stdlib.h>

using namespace std;
using namespace workrave::utils;

bool
Platform::registry_get_value(const char *path, const char *name,
                         char *out)
{
  HKEY handle;
  bool rc = false;
  LONG err;

  err = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      size = MAX_PATH;
      err = RegQueryValueExA(handle, name, 0, &type, (LPBYTE) out, &size);
      if (err == ERROR_SUCCESS)
        {
          rc = true;
        }
      RegCloseKey(handle);
    }
  return rc;
}

bool
Platform::registry_set_value(const char *path, const char *name,
                         const char *value)
{
  HKEY handle;
  bool rc = false;
  DWORD disp;
  LONG err;

  err = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      if (value != NULL)
        {
          err = RegSetValueEx(handle, name, 0, REG_SZ, (BYTE *) value,
                              strlen(value)+1);
        }
      else
        {
          err = RegDeleteValue(handle, name);
        }
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  return rc;
}

//! Returns the directory in which workrave is installed.
std::string
Platform::get_application_directory()
{
  char app_dir_name[MAX_PATH];
  GetModuleFileName(GetModuleHandle(NULL), app_dir_name, sizeof(app_dir_name));
  // app_dir_name == c:\program files\workrave\lib\workrave.exe
  char *s = strrchr(app_dir_name, '\\');
  assert (s);
  *s = '\0';
  // app_dir_name == c:\program files\workrave\lib
  s = strrchr(app_dir_name, '\\');
  assert (s);
  *s = '\0';
  // app_dir_name == c:\program files\workrave
  return string(app_dir_name);
}

std::string
Platform::get_application_name()
{
  char app_dir_name[MAX_PATH];
  GetModuleFileName(GetModuleHandle(NULL), app_dir_name, sizeof(app_dir_name));
  // app_dir_name == c:\program files\workrave\lib\workrave.exe
  char *s = strrchr(app_dir_name, '\\');
  return string(s);
}

std::wstring
Platform::convert(const char* c)
{
   std::string s(c);
   
   return std::wstring(s.begin(), s.end());
}

int
Platform::setenv(const char* name, const char* val, int) 
{
  return SetEnvironmentVariableW(convert(name).c_str(), convert(val).c_str()); 
}

int
Platform::unsetenv(const char* name) 
{ 
  return SetEnvironmentVariableW(convert(name).c_str(), 0); 
}

