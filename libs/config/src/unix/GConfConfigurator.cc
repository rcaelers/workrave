// GConfConfigurator.cc --- Configuration Access
//
// Copyright (C) 2002, 2003, 2006, 2007, 2013 Rob Caelers <robc@krandor.nl>
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

#include <sstream>
#include <assert.h>

#include <gconf/gconf-client.h>
#define GCONF_HACK

#include "GConfConfigurator.hh"
#include "Configurator.hh"

using namespace workrave;
using namespace std;

GConfConfigurator::GConfConfigurator()
{
  gconf_client = gconf_client_get_default();
  gconf_root = "/apps/workrave";

#ifndef NDEBUG
  const char *env = getenv("WORKRAVE_GCONF_ROOT");
  if (env != NULL)
    {
      gconf_root = env;
    }
  int len = gconf_root.length();
  if (len > 0)
    {
      if (gconf_root[len - 1] == '/')
        {
          gconf_root = gconf_root.substr(0, len - 1);
        }
    }
#endif

  GError *error = NULL;
  gconf_client_add_dir(gconf_client, gconf_root.c_str(), GCONF_CLIENT_PRELOAD_NONE, &error);
  if (error != NULL)
    {
      g_error_free(error);
    }
}


GConfConfigurator::~GConfConfigurator()
{
  GError *error = NULL;

  for (IDMapIter i = ids.begin(); i != ids.end(); i++)
    {
      guint id = i->first;
      gconf_client_notify_remove(gconf_client, id);
    }

  gconf_client_remove_dir(gconf_client, gconf_root.c_str(), &error);

  if (error != NULL)
    {
      g_error_free(error);
    }
}


bool
GConfConfigurator::load(string filename)
{
  (void) filename;
  return true;
}


bool
GConfConfigurator::save(string filename)
{
  (void) filename;
  return true;
}


bool
GConfConfigurator::save()
{
  return true;
}


bool
GConfConfigurator::get_value(const string &key, GConfValue **value) const
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + "/" + key;

  assert(value != NULL);
  *value  = gconf_client_get_without_default(gconf_client, full_key.c_str(), &error);

  if (error != NULL || *value == NULL)
    {
      ret = false;
      if (*value != NULL)
        {
          gconf_value_free(*value);
          *value = NULL;
        }
      if (error != NULL)
        {
          g_error_free(error);
        }
    }


  return ret;
}


bool
GConfConfigurator::remove_key(const std::string &key)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + "/" + key;

  ret = gconf_client_unset(gconf_client, full_key.c_str(), &error);

  if (error != NULL)
    {
      g_error_free(error);
      ret = false;
    }

  return ret;
}


bool
GConfConfigurator::get_value(const std::string &key, VariantType type,
                             Variant &out) const
{
  bool ret = false;
  GConfValue *value;

  ret = get_value(key, &value);

  if (ret)
    {
      if (type == VARIANT_TYPE_NONE)
        {
          switch (value->type)
            {
            case GCONF_VALUE_INT:
              type = VARIANT_TYPE_INT;
              break;

            case GCONF_VALUE_BOOL:
              type = VARIANT_TYPE_BOOL;
              break;

            case GCONF_VALUE_FLOAT:
              type = VARIANT_TYPE_DOUBLE;
              break;

            case GCONF_VALUE_STRING:
              type = VARIANT_TYPE_STRING;
              break;

            default:
              break;
            }
        }


      ret = false;
      switch(type)
        {
        case VARIANT_TYPE_INT:
          if (value->type == GCONF_VALUE_INT)
            {
              out.int_value = gconf_value_get_int(value);
              ret = true;
            }
          break;

        case VARIANT_TYPE_BOOL:
          if (value->type == GCONF_VALUE_BOOL)
            {
              out.bool_value = gconf_value_get_bool(value);
              ret = true;
            }
          break;

        case VARIANT_TYPE_DOUBLE:
          if (value->type == GCONF_VALUE_FLOAT)
            {
              out.double_value = gconf_value_get_float(value);
              ret = true;
            }
          break;

        case VARIANT_TYPE_STRING:
          if (value->type == GCONF_VALUE_STRING)
            {
              out.string_value = gconf_value_get_string(value);
              ret = true;
            }
          break;

        case VARIANT_TYPE_NONE:
        default:
          ret = false;
        }

      gconf_value_free(value);
    }

  if (ret)
    {
      out.type = type;
    }

  return ret;
}


bool
GConfConfigurator::set_value(const std::string &key, Variant &value)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + "/" + key;

  switch(value.type)
    {
    case VARIANT_TYPE_NONE:
      ret = false;
      break;

    case VARIANT_TYPE_INT:
      ret = gconf_client_set_int(gconf_client, full_key.c_str(),
                                 value.int_value, &error);
      break;

    case VARIANT_TYPE_BOOL:
      ret = gconf_client_set_bool(gconf_client, full_key.c_str(),
                                  value.bool_value, &error);
      break;

    case VARIANT_TYPE_DOUBLE:
      ret = gconf_client_set_float(gconf_client, full_key.c_str(),
                                   value.double_value, &error);
      break;

    case VARIANT_TYPE_STRING:
      ret = gconf_client_set_string(gconf_client, full_key.c_str(),
                                    value.string_value.c_str(),
                                    &error);
      break;

    default:
      ret = false;
    }

  if (error != NULL)
    {
      ret = false;
      if (error != NULL)
        {
          g_error_free(error);
        }
    }
  return ret;
}



void
GConfConfigurator::set_listener(IConfiguratorListener *listener)
{
  this->listener = listener;
}


bool
GConfConfigurator::add_listener(const string &key_prefix)
{
  TRACE_ENTER_MSG("GConfConfigurator::add_listener", key_prefix);

  string full_key = gconf_root + "/" + key_prefix;
  GError *error = NULL;
  guint id = 0;

  int len = full_key.length();
  if (len > 0)
    {
      if (full_key[len - 1] == '/')
        {
          full_key = full_key.substr(0, len - 1);
        }
    }

  // Add notification callback.

  id = gconf_client_notify_add(gconf_client,
                               full_key.c_str(),
                               &GConfConfigurator::static_key_changed,
                               (gpointer)this,
                               NULL,
                               &error);

  if (error != NULL)
    {
      g_error_free(error);
    }
  else
    {
      ids[id] = key_prefix;
    }

  TRACE_EXIT();
  return error == NULL;
}


bool
GConfConfigurator::remove_listener(const string &remove_key)
{
  bool ret = false;

  IDMapIter i = ids.begin();
  while (i != ids.end())
    {
      IDMapIter next = i;
      next++;

      string &key = i->second;

      if (key == remove_key)
        {
          guint id = i->first;
          gconf_client_notify_remove(gconf_client, id);

          ids.erase(i);
          ret = true;
        }

      i = next;
    }

  return ret;
}


void
GConfConfigurator::static_key_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data)
{
  (void)client;
  GConfConfigurator *c = (GConfConfigurator *) user_data;
  c->key_changed(cnxn_id, entry);
}


void
GConfConfigurator::key_changed(guint id, GConfEntry *entry)
{
  TRACE_ENTER_MSG("GConfConfigurator::key_changed", id);
  (void) entry;

  IDMapIter i = ids.find(id);
  if (i != ids.end())
    {
      listener->config_changed_notify(i->second);
    }

  TRACE_EXIT();
}
