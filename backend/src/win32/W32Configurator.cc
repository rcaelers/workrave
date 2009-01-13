// W32Configurator.cc --- Configuration Access
//
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "debug.hh"
#include "W32Configurator.hh"

using namespace std;

W32Configurator::W32Configurator()
{
  key_root = "Software/Workrave";
}


W32Configurator::~W32Configurator()
{
}


bool
W32Configurator::load(string filename)
{
  (void) filename;
  return true;
}


bool
W32Configurator::save(string filename)
{
  (void) filename;
  return true;
}


bool
W32Configurator::save()
{
  return true;
}


bool
W32Configurator::remove_key(const std::string &key)
{
  TRACE_ENTER_MSG("W32Configurator::remove_key", key);

  HKEY handle;
  bool rc = false;
  string k, p, p32, c;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD size;
      char buf[256]; // FIXME: yuck, should be dynamic.
      size = sizeof(buf);
      err = RegDeleteKey(handle, c.c_str());
      if (err == ERROR_SUCCESS)
        {
          rc = true;
        }
      RegCloseKey(handle);
    }

  TRACE_EXIT();
  return rc;
}


bool
W32Configurator::get_value(const string &key, string &out) const
{
  TRACE_ENTER_MSG("W32Configurator::get_value", key << "," << out);

  HKEY handle;
  bool rc = false;
  string k, p, p32, c;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);

  err = RegOpenKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);

  if (err == ERROR_SUCCESS)
    {
      DWORD size, type;
      char *buffer;

      // get the size, in bytes, required for buffer
      err = RegQueryValueExA( handle, c.c_str(), NULL, NULL, NULL, &size );

      if (err != ERROR_SUCCESS || !size)
        {
          RegCloseKey( handle );
          TRACE_EXIT();
          return false;
        }
      else if( !( buffer = (char *)malloc( size + 1 ) ) )
        {
          RegCloseKey( handle );
          TRACE_EXIT();
          return false;
        }

      err = RegQueryValueExA( handle, c.c_str(), NULL, &type, (LPBYTE)buffer, &size );
      buffer[ size ] = '\0';

      if ( err == ERROR_SUCCESS && type == REG_SZ )
        {
          out = buffer;
          rc = true;
        }

      RegCloseKey( handle );
      free( buffer );
    }

  TRACE_EXIT();
  return rc;
}


bool
W32Configurator::get_value(const string &key, bool &out) const
{
  long l;
  bool rc = get_value(key, l);
  if (rc)
    {
      out = l ? true : false;
    }
  return rc;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
W32Configurator::get_value(const string &key, int &out) const
{
  long l;
  bool rc = get_value(key, l);
  if (rc)
    {
      out = (int) l;
    }
  return rc;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
W32Configurator::get_value(const string &key, long &out) const
{
  string s;
  bool rc = get_value(key, s);
  if (rc)
    {
      int f = sscanf(s.c_str(), "%ld", &out);
      rc = (f == 1);
    }
  return rc;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
W32Configurator::get_value(const string &key, double &out) const
{
  string s;
  bool rc = get_value(key, s);
  if (rc)
    {
      int f = sscanf(s.c_str(), "%lf", &out);
      rc = (f == 1);
    }
  return rc;
}


bool
W32Configurator::set_value(const string &key, string v)
{
  TRACE_ENTER_MSG("W32Configurator::set_value", key << "," << v);

  HKEY handle;
  bool rc = false;
  string k, p, p32, c;
  DWORD disp;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);
  err = RegCreateKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      err = RegSetValueEx(handle, c.c_str(), 0, REG_SZ, (BYTE *) v.c_str(), v.length()+1);
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  TRACE_EXIT();
  return rc;
}




bool
W32Configurator::set_value(const string &key, int v)
{
  char buf[32];
  sprintf(buf, "%d", v);
  return set_value(key, string(buf));
}

bool
W32Configurator::set_value(const string &key, long v)
{
  char buf[32];
  sprintf(buf, "%ld", v);
  return set_value(key, string(buf));
}

bool
W32Configurator::set_value(const string &key, bool v)
{
  char buf[32];
  sprintf(buf, "%d", v ? 1 : 0);
  return set_value(key, string(buf));
}


bool
W32Configurator::set_value(const string &key, double v)
{
  char buf[32];
  sprintf(buf, "%f", v);
  return set_value(key, string(buf));
}



string
W32Configurator::key_add_part(string s, string t) const
{
  string ret = s;
  add_trailing_slash(ret);
  return ret + t;
}

void
W32Configurator::key_split(const string &key, string &parent, string &child) const
{
  const char *s = key.c_str();
  const char *slash = strrchr(s, '/');
  if (slash)
    {
      parent = key.substr(0, slash-s);
      child = slash+1;
    }
  else
    {
      parent = "";
      child = "";
    }
}

string
W32Configurator::key_win32ify(const string &key) const
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
W32Configurator::strip_trailing_slash(string &key) const
{
  int len = key.length();
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
W32Configurator::add_trailing_slash(string &key) const
{
  int len = key.length();
  if (len > 0)
    {
      if (key[len - 1] != '/')
        {
          key += '/';
        }
    }
}
