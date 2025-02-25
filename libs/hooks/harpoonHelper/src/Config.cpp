// Copyright (C) 2002, 2005, 2006, 2007, 2009 Raymond Penners <raymond@dotsphinx.com>
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

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include "Debug.h"
#include "Config.h"

using namespace std;

Config::Config()
{
  key_root = "Software/Workrave";
}

bool
Config::get_value(const string &key, string &out) const
{
  TRACE_ENTER_MSG("Config::get_value", key << "," << out);

  HKEY handle;
  bool rc = false;
  string k, p, p32, c;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_windowsify(p);

  err = RegOpenKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);

  if (err == ERROR_SUCCESS)
    {
      DWORD size, type;
      char *buffer;

      // get the size, in bytes, required for buffer
      err = RegQueryValueExA(handle, c.c_str(), NULL, NULL, NULL, &size);

      if (err != ERROR_SUCCESS || !size)
        {
          RegCloseKey(handle);
          TRACE_EXIT();
          return false;
        }
      else if (!(buffer = (char *)malloc(size + 1)))
        {
          RegCloseKey(handle);
          TRACE_EXIT();
          return false;
        }

      err = RegQueryValueExA(handle, c.c_str(), NULL, &type, (LPBYTE)buffer, &size);
      buffer[size] = '\0';

      if (err == ERROR_SUCCESS && type == REG_SZ)
        {
          out = buffer;
          rc = true;
        }

      RegCloseKey(handle);
      free(buffer);
    }

  TRACE_EXIT();
  return rc;
}

//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
Config::get_value(const string &key, int &out) const
{
  string s;
  bool rc = get_value(key, s);
  if (rc)
    {
      int f = sscanf(s.c_str(), "%d", &out);
      rc = (f == 1);
    }
  return rc;
}

bool
Config::get_value(const string &key, bool &out) const
{
  int l;
  bool rc = get_value(key, l);
  if (rc)
    {
      out = l ? true : false;
    }
  return rc;
}

string
Config::key_add_part(string s, string t) const
{
  string ret = s;
  add_trailing_slash(ret);
  return ret + t;
}

void
Config::key_split(const string &key, string &parent, string &child) const
{
  const char *s = key.c_str();
  const char *slash = strrchr(s, '/');
  if (slash)
    {
      parent = key.substr(0, slash - s);
      child = slash + 1;
    }
  else
    {
      parent = "";
      child = "";
    }
}

string
Config::key_windowsify(const string &key) const
{
  string rc = key;
  strip_trailing_slash(rc);
  for (unsigned int i = 0; i < rc.length(); i++)
    {
      if (rc[i] == '/')
        {
          rc[i] = '\\';
        }
    }
  return rc;
}

//! Removes the trailing '/'.
void
Config::strip_trailing_slash(string &key) const
{
  size_t len = key.length();
  if (len > 0)
    {
      if (key[len - 1] == '/')
        {
          key = key.substr(0, len - 1);
        }
    }
}

//! Adds add trailing '/' if it isn't there yet.
void
Config::add_trailing_slash(string &key) const
{
  size_t len = key.length();
  if (len > 0)
    {
      if (key[len - 1] != '/')
        {
          key += '/';
        }
    }
}
