// Win32Configurator.cc --- Configuration Access
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "debug.hh"
#include "Win32Configurator.hh"

Win32Configurator::Win32Configurator()
{
  TRACE_ENTER("Win32Configurator::Win32Configurator");
  key_root = "Software/Workrave";
}


Win32Configurator::~Win32Configurator()
{
  TRACE_ENTER("Win32Configurator::~Win32Configurator");
}


bool
Win32Configurator::load(string filename)
{
  return true;
}


bool
Win32Configurator::save(string filename)
{
  return true;
}


bool
Win32Configurator::save()
{
  return true;
}


bool
Win32Configurator::get_value(string key, string *out) const
{
  TRACE_ENTER_MSG("Win32Configurator::get_value", key << "," << out);

  HKEY handle;
  bool rc = false;
  string k, p, p32, c;
  DWORD disp;
  LONG err;

  k = key_add_part(key_root, key);
  key_split(k, p, c);
  p32 = key_win32ify(p);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, p32.c_str(), 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      char buf[256]; // FIXME: yuck, should be dynamic.
      size = sizeof(buf);
      err = RegQueryValueEx(handle, c.c_str(), 0, &type, (LPBYTE) buf, &size);
      if (err == ERROR_SUCCESS)
        {
          *out = buf;
          rc = true;
        }
      RegCloseKey(handle);
    }
  return rc;
}


bool
Win32Configurator::get_value(string key, bool *out) const
{
  long l;
  bool rc = get_value(key, &l);
  if (rc)
    {
      *out = (bool) l;
    }
  return rc;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
Win32Configurator::get_value(string key, int *out) const
{
  long l;
  bool rc = get_value(key, &l);
  if (rc)
    {
      *out = (int) l;
    }
  return rc;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
Win32Configurator::get_value(string key, long *out) const
{
  string s;
  bool rc = get_value(key, &s);
  if (rc)
    {
      int f = sscanf(s.c_str(), "%ld", out);
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
Win32Configurator::get_value(string key, double *out) const
{
  string s;
  bool rc = get_value(key, &s);
  if (rc)
    {
      int f = sscanf(s.c_str(), "%f", out);
      rc = (f == 1);
    }
  return rc;
}


bool
Win32Configurator::set_value(string key, string v)
{
  TRACE_ENTER_MSG("Win32Configurator::set_value", key << "," << v);

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
      if (rc)
	{
	  fire_configurator_event(key);
	}
    }
  return rc;
}




bool
Win32Configurator::set_value(string key, int v)
{
  char buf[32];
  sprintf(buf, "%d", v);
  return set_value(key, string(buf));
}

bool
Win32Configurator::set_value(string key, long v)
{
  char buf[32];
  sprintf(buf, "%ld", v);
  return set_value(key, string(buf));
}

bool
Win32Configurator::set_value(string key, bool v)
{
  char buf[32];
  sprintf(buf, "%d", v ? 1 : 0);
  return set_value(key, string(buf));
}


bool
Win32Configurator::set_value(string key, double v)
{
  char buf[32];
  sprintf(buf, "%f", v);
  return set_value(key, string(buf));
}


bool
Win32Configurator::exists_dir(string key) const
{
  TRACE_ENTER_MSG("Win32Configurator::exists_dir", key);
  HKEY handle;
  bool rc = false;
  string k, k32;
  DWORD disp;
  LONG err;

  k = key_add_part(key_root, key);
  k32 = key_win32ify(k);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, k32.c_str(), 0, KEY_READ, &handle);
  if (err == ERROR_SUCCESS)
    {
      rc = true;
      RegCloseKey(handle);
    }
  return rc;
}






list<string>
Win32Configurator::get_all_dirs(string key) const
{
  TRACE_ENTER_MSG("Win32Configurator::get_all_dirs", key);
  list<std::string> ret;
  LONG err;
  HKEY handle;

  string k = key_add_part(key_root, key);
  string k32 = key_win32ify(k);
  err = RegOpenKeyEx(HKEY_CURRENT_USER, k32.c_str(), 0,
		     KEY_ALL_ACCESS, &handle);
  TRACE_MSG(k32);
  if (err == ERROR_SUCCESS)
    {
      for (int i = 0; ; i++)
	{
	  // FIXME: static length, eek
	  char buf[256];
	  DWORD buf_size = sizeof(buf);
	  FILETIME time;
      
	  err = RegEnumKeyEx(handle, i, buf, &buf_size, NULL, NULL, NULL, &time);
	  if (err == ERROR_SUCCESS)
	    {
	      string s = buf;
	      ret.push_back(s);
	    }
	  else
	    {
	      break;
	    }
	}
      RegCloseKey(handle);
    }
  return ret;  
}


string
Win32Configurator::key_add_part(string s, string t) const
{
  string ret = s;
  add_trailing_slash(ret);
  return ret + t;
}

void
Win32Configurator::key_split(string key, string &parent, string &child) const
{
  const char *s = key.c_str();
  char *slash = strrchr(s, '/');
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
Win32Configurator::key_win32ify(string key) const
{
  string rc = key;
  strip_trailing_slash(rc);
  for (int i = 0; i < rc.length(); i++)
    {
      if (rc[i] == '/')
        {
          rc[i] = '\\';
        }
    }
  return rc;
}


