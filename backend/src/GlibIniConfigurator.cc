// IniConfigurator.cc --- Configuration Access
//
// Copyright (C) 2005, 2006, 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include <string.h>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <fstream>

#include "GlibIniConfigurator.hh"
#include <glib.h>

using namespace std;

GlibIniConfigurator::GlibIniConfigurator()
  : config(NULL)
{
}


GlibIniConfigurator::~GlibIniConfigurator()
{
  if (config != NULL)
    {
      g_key_file_free(config);
    }
}


bool
GlibIniConfigurator::load(string filename)
{
  GError *error = NULL;
  gboolean r = TRUE;

  last_filename = filename;

  TRACE_ENTER_MSG("GlibIniConfigurator::load", filename)
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
GlibIniConfigurator::save(string filename)
{
  GError *error = NULL;
  char *str = g_key_file_to_data(config, NULL, &error);

  TRACE_ENTER_MSG("GlibIniConfigurator::save", filename);
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

  if (str != NULL)
    {
      g_free(str);
    }

  TRACE_EXIT();
  return error == NULL;
}


bool
GlibIniConfigurator::save()
{
  return save(last_filename);
}


bool
GlibIniConfigurator::remove_key(const std::string &key)
{
  bool ret = true;
  GError *error = NULL;
  string group;
  string inikey;

  TRACE_ENTER_MSG("GlibIniConfigurator::remove_key", key);
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  g_key_file_remove_key(config, group.c_str(), inikey.c_str(), &error);

  if (error != NULL)
    {
      g_error_free(error);
      ret = false;
    }

  TRACE_EXIT();
  return ret;
}


bool
GlibIniConfigurator::get_value(const std::string &key, VariantType type,
                               Variant &out) const
{
  bool ret = true;
  GError *error = NULL;
  string group;
  string inikey;

  TRACE_ENTER_MSG("GlibIniConfigurator::get_value", key);
  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  out.type = type;

  switch(type)
    {
    case VARIANT_TYPE_INT:
      out.int_value = g_key_file_get_integer(config, group.c_str(), inikey.c_str(), &error);
      break;

    case VARIANT_TYPE_BOOL:
      out.bool_value = g_key_file_get_boolean(config, group.c_str(), inikey.c_str(), &error);
      break;

    case VARIANT_TYPE_DOUBLE:
      {
        char *s = g_key_file_get_string(config, group.c_str(), inikey.c_str(), &error);
        if (error == NULL && s != NULL)
          {
            sscanf(s, "%lf", &out.double_value);
          }
        break;
      }

    case VARIANT_TYPE_NONE:
      out.type = VARIANT_TYPE_STRING;
      // FALLTHROUGH

    case VARIANT_TYPE_STRING:
      {
        char *s = g_key_file_get_string(config, group.c_str(), inikey.c_str(), &error);
        if (error == NULL && s != NULL)
          {
            out.string_value = s;
          }
      }
      break;

    default:
      ret = false;
    }

  if (error != NULL)
    {
      g_error_free(error);
      ret = false;
    }

  TRACE_EXIT();
  return ret;
}

bool
GlibIniConfigurator::set_value(const std::string &key, Variant &value)
{
  bool ret = true;
  string group;
  string inikey;

  split_key(key, group, inikey);
  inikey = key_inify(inikey);

  switch(value.type)
    {
    case VARIANT_TYPE_INT:
      g_key_file_set_integer(config, group.c_str(), inikey.c_str(), value.int_value);
      break;

    case VARIANT_TYPE_BOOL:
      g_key_file_set_boolean(config, group.c_str(), inikey.c_str(), value.bool_value);
      break;

    case VARIANT_TYPE_DOUBLE:
      {
        char buf[32];
        sprintf(buf, "%f", value.double_value);

        split_key(key, group, inikey);
        inikey = key_inify(inikey);

        g_key_file_set_string(config, group.c_str(), inikey.c_str(), buf);
      }
      break;

    case VARIANT_TYPE_NONE:
    case VARIANT_TYPE_STRING:
      g_key_file_set_string(config, group.c_str(), inikey.c_str(), value.string_value.c_str());
      break;

    default:
      ret = false;
    }

  return ret;
}


void
GlibIniConfigurator::split_key(const string &key, string &group, string &out_key) const
{
  const char *s = key.c_str();
  const char *slash = strchr(s, '/');
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
GlibIniConfigurator::key_inify(const string &key) const
{
  string rc = key;
  for (unsigned int i = 0; i < rc.length(); i++)
    {
      if (rc[i] == '/')
        {
          rc[i] = '.';
        }
    }
  return rc;
}
