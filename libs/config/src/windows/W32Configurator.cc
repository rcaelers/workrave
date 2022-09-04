// Copyright (C) 2002 - 2012 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include <stdio.h>
#include <stdlib.h>
#include "boost/lexical_cast.hpp"

#include "debug.hh"
#include "W32Configurator.hh"

W32Configurator::W32Configurator()
{
  key_root = "Software/Workrave";
}

bool
W32Configurator::load(std::string filename)
{
  (void)filename;
  return true;
}

void
W32Configurator::save()
{
}

bool
W32Configurator::has_user_value(const std::string &key)
{
  std::optional<ConfigValue> v = get_value(key, ConfigType::Unknown);
  return v.has_value();
}

void
W32Configurator::remove_key(const std::string &key)
{
  TRACE_ENTRY_PAR(key);

  HKEY handle;
  std::string k, p, p32, c;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_windowsify(p);

  err = RegOpenKeyExA(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      err = RegDeleteValueA(handle, c.c_str());
      if (err != ERROR_SUCCESS)
        {
          // FIXME: Log error
        }
      RegCloseKey(handle);
    }
}

std::optional<ConfigValue>
W32Configurator::get_value(const std::string &key, ConfigType type) const
{
  std::string k = key_add_part(key_root, key);
  std::string p, c;
  key_split(k, p, c);
  std::string p32 = key_windowsify(p);

  bool rc = false;
  HKEY handle;
  LONG err = RegOpenKeyExA(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);

  std::string value;

  if (err == ERROR_SUCCESS)
    {
      DWORD size, type;
      char *buffer;

      // get the size, in bytes, required for buffer
      err = RegQueryValueExA(handle, c.c_str(), NULL, NULL, NULL, &size);

      if (err != ERROR_SUCCESS || !size)
        {
          RegCloseKey(handle);
          return {};
        }
      else if (!(buffer = (char *)malloc(size + 1)))
        {
          RegCloseKey(handle);
          return {};
        }

      err = RegQueryValueExA(handle, c.c_str(), NULL, &type, (LPBYTE)buffer, &size);
      buffer[size] = '\0';

      if (err == ERROR_SUCCESS && type == REG_SZ)
        {
          value = buffer;
          rc = true;
        }

      RegCloseKey(handle);
      free(buffer);
    }

  if (rc)
    {
      switch (type)
        {
        case ConfigType::Int32:
          return boost::lexical_cast<int32_t>(value);

        case ConfigType::Int64:
          return boost::lexical_cast<int64_t>(value);

        case ConfigType::Boolean:
          return boost::lexical_cast<bool>(value);

        case ConfigType::Double:
          return boost::lexical_cast<double>(value);

        case ConfigType::Unknown:
          [[fallthrough]];

        case ConfigType::String:
          return value;
        }
    }
  return {};
}

void
W32Configurator::set_value(const std::string &key, const ConfigValue &value)
{
  std::visit(
    [this, key](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      std::string v;

      if constexpr (std::is_same_v<std::string, T>)
        {
          v = arg;
        }
      else if constexpr (!std::is_same_v<std::monostate, T>)
        {
          v = std::to_string(arg);
        }

      std::string k = key_add_part(key_root, key);
      std::string p, c;
      key_split(k, p, c);
      std::string p32 = key_windowsify(p);

      HKEY handle;
      DWORD disp;
      LONG err = RegCreateKeyExA(HKEY_CURRENT_USER,
                                 p32.c_str(),
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS,
                                 NULL,
                                 &handle,
                                 &disp);
      if (err == ERROR_SUCCESS)
        {
          err = RegSetValueExA(handle, c.c_str(), 0, REG_SZ, (BYTE *)v.c_str(), static_cast<DWORD>(v.length() + 1));
          RegCloseKey(handle);
          // TODO: Log rc = (err == ERROR_SUCCESS);
        }
    },
    value);
}

std::string
W32Configurator::key_add_part(std::string s, std::string t) const
{
  std::string ret = s;
  add_trailing_slash(ret);
  return ret + t;
}

void
W32Configurator::key_split(const std::string &key, std::string &parent, std::string &child) const
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

std::string
W32Configurator::key_windowsify(const std::string &key) const
{
  std::string rc = key;
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

void
W32Configurator::strip_trailing_slash(std::string &key) const
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

void
W32Configurator::add_trailing_slash(std::string &key) const
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
