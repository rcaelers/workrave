// IniConfigurator.cc --- Configuration Access
//
// Copyright (C) 2005, 2006 Rob Caelers <robc@krandor.org>
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

#include "debug.hh"

#include <sstream>
#include <assert.h>
#include <iostream>
#include <fstream>

#include "IniConfigurator.hh"
#include <glib.h>

IniConfigurator::IniConfigurator()
  : config(NULL)
{
}


IniConfigurator::~IniConfigurator()
{
  if (config != NULL)
    {
      g_key_file_free(config);
    }
}


bool
IniConfigurator::load(string filename)
{
  GError *error = NULL;
  gboolean r = TRUE;

  last_filename = filename;
  
  TRACE_ENTER_MSG("IniConfigurator::load", filename)
  config = g_key_file_new();

  r = g_key_file_load_from_file(config, filename.c_str(),
                                G_KEY_FILE_KEEP_COMMENTS, &error);

  if (r)
    {
    }

  if (error != NULL)
    {
      g_error_free(error);
    }

  TRACE_EXIT();
  return error == NULL;
}


bool
IniConfigurator::save(string filename)
{
  GError *error = NULL;
  char *str = g_key_file_to_data(config, NULL, &error);

  TRACE_ENTER_MSG("IniConfigurator::save", filename);
  if (error != NULL)
    {
      g_error_free(error);
    }
  else
    {
      ofstream config_file(filename.c_str());
      
      config_file << str;
      
      config_file.close();
    }

  TRACE_EXIT();
  return error == NULL;
}


bool
IniConfigurator::save()
{
  return save(last_filename);
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
IniConfigurator::get_value(string key, string *out) const
{
  GError *error = NULL;
  string group;
  string inikey;
  gchar *value;

  TRACE_ENTER_MSG("IniConfigurator::get_value", key);
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  value = g_key_file_get_string(config, group.c_str(), inikey.c_str(), &error);

  if (value != NULL)
    {
      *out = value;
    }
  
  if (error != NULL)
    {
      TRACE_MSG("error");
      g_error_free(error);
    }

  TRACE_MSG(*out);
  TRACE_EXIT();
  return (error == NULL);
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
IniConfigurator::get_value(string key, bool *out) const
{
  GError *error = NULL;
  string group;
  string inikey;
  gboolean value;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  value = g_key_file_get_boolean(config, group.c_str(), inikey.c_str(), &error);

  if (error != NULL)
    {
      g_error_free(error);
    }
  else
    {
      *out = value;
    }

  return (error == NULL);
}



//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
IniConfigurator::get_value(string key, int *out) const
{
  GError *error = NULL;
  string group;
  string inikey;
  gint value;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  value = g_key_file_get_integer(config, group.c_str(), inikey.c_str(), &error);
  
  if (error != NULL)
    {
      g_error_free(error);
      *out = 0;
    }
  else
    {
      *out = value;
    }

  return (error == NULL);
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
IniConfigurator::get_value(string key, long *out) const
{
  int value = 0;
  bool ret = get_value(key, &value);

  *out = value;
  return ret;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
IniConfigurator::get_value(string key, double *out) const
{
  GError *error = NULL;
  string group;
  string inikey;
  char *value;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  value = g_key_file_get_string(config, group.c_str(), inikey.c_str(), &error);
  
  if (error != NULL)
    {
      g_error_free(error);
    }
  else
    {
      int f = sscanf(value, "%lf", out);
      (void) f;
    }

  return (error == NULL);
}


bool
IniConfigurator::set_value(string key, string v)
{
  string group;
  string inikey;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_set_string(config, group.c_str(), inikey.c_str(), v.c_str());

  return true;
}


bool
IniConfigurator::set_value(string key, int v)
{
  string group;
  string inikey;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_set_integer(config, group.c_str(), inikey.c_str(), v);

  return true;
}

bool
IniConfigurator::set_value(string key, long v)
{
  string group;
  string inikey;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_set_integer(config, group.c_str(), inikey.c_str(), v);

  return true;
}

bool
IniConfigurator::set_value(string key, bool v)
{
  string group;
  string inikey;
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_set_boolean(config, group.c_str(), inikey.c_str(), v);

  return true;
}


bool
IniConfigurator::set_value(string key, double v)
{
  string group;
  string inikey;

  char buf[32];
  sprintf(buf, "%f", v);
  
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_set_string(config, group.c_str(), inikey.c_str(), buf);

  return true;
}



bool
IniConfigurator::exists_dir(string key) const
{
  (void) key;
  return false;
}


list<string>
IniConfigurator::get_all_dirs(string key) const
{
  list<string> l;
  
  (void) key;
  
  return l;
}


void
IniConfigurator::split_key(const string key, string &group, string &out_key) const
{
  const char *s = key.c_str();
  char *slash = strchr(s, '/');
  if (slash)
    {
      group = key.substr(0, slash-s);
      out_key = slash+1;
    }
  else
    {
      group = "";
      out_key = "";
    }
}


string
IniConfigurator::key_inify(string key) const
{
  string rc = key;
  strip_trailing_slash(rc);
  for (unsigned int i = 0; i < rc.length(); i++)
    {
      if (rc[i] == '/')
        {
          rc[i] = '.';
        }
    }
  return rc;
}
