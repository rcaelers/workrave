// GConfConfigurator.cc --- Configuration Access
//
// Copyright (C) 2002, 2003, 2006 Rob Caelers <robc@krandor.org>
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

#include <gconf/gconf-client.h>
#define GCONF_HACK

#include "GConfConfigurator.hh"


GConfConfigurator::GConfConfigurator()
{
  gconf_client = gconf_client_get_default();
  gconf_root = "/apps/workrave/";

#ifndef NDEBUG
  const char *env = getenv("WORKRAVE_GCONF_ROOT");
  if (env != NULL)
    {
      gconf_root = env;
    }
#endif
}


GConfConfigurator::~GConfConfigurator()
{
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
GConfConfigurator::get_value(string key, GConfValue **value) const
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;

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
    }

  return ret;
}



//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
GConfConfigurator::get_value(string key, string *out) const
{
  GConfValue *value;

  bool ret = get_value(key, &value);
  if (ret)
    {
      if (value->type == GCONF_VALUE_STRING)
        {
          *out = gconf_value_get_string(value);
        }
      else
        {
          ret = false;
        }
      gconf_value_free(value);
    }
  return ret;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
GConfConfigurator::get_value(string key, bool *out) const
{
  GConfValue *value;

  bool ret = get_value(key, &value);
  if (ret)
    {
      if (value->type == GCONF_VALUE_BOOL)
        {
          *out = gconf_value_get_bool(value);
        }
      else
        {
          ret = false;
        }
      gconf_value_free(value);
    }
  return ret;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
GConfConfigurator::get_value(string key, int *out) const
{
  GConfValue *value;

  bool ret = get_value(key, &value);
  if (ret)
    {
      if (value->type == GCONF_VALUE_INT)
        {
          *out = gconf_value_get_int(value);
        }
      else
        {
          ret = false;
        }
      gconf_value_free(value);
    }
  return ret;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
GConfConfigurator::get_value(string key, long *out) const
{
  GConfValue *value;

  bool ret = get_value(key, &value);
  if (ret)
    {
      if (value->type == GCONF_VALUE_INT)
        {
          *out = gconf_value_get_int(value);
        }
      else
        {
          ret = false;
        }
      gconf_value_free(value);
    }
  return ret;
}


//! Returns the value of the specified attribute
/*!
 *  \retval true value successfully returned.
 *  \retval false attribute not found.
 */
bool
GConfConfigurator::get_value(string key, double *out) const
{
  GConfValue *value;

  bool ret = get_value(key, &value);
  if (ret)
    {
      if (value->type == GCONF_VALUE_FLOAT)
        {
          *out = gconf_value_get_float(value);
        }
      else
        {
          ret = false;
        }
      gconf_value_free(value);
    }
  return ret;
}


bool
GConfConfigurator::set_value(string key, string v)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;
  ret = gconf_client_set_string(gconf_client, full_key.c_str(), v.c_str(), &error);

  if (error != NULL)
    {
      ret = false;
    }
  return ret;
}


bool
GConfConfigurator::set_value(string key, int v)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;
  ret = gconf_client_set_int(gconf_client, full_key.c_str(), v, &error);

  if (error != NULL)
    {
      ret = false;
    }
  return ret;

}

bool
GConfConfigurator::set_value(string key, long v)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;
  ret = gconf_client_set_int(gconf_client, full_key.c_str(), v, &error);

  if (error != NULL)
    {
      ret = false;
    }
  return ret;
}

bool
GConfConfigurator::set_value(string key, bool v)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;
  ret = gconf_client_set_bool(gconf_client, full_key.c_str(), v, &error);

  if (error != NULL)
    {
      ret = false;
    }
  return ret;
}


bool
GConfConfigurator::set_value(string key, double v)
{
  bool ret = true;
  GError *error = NULL;

  string full_key = gconf_root + key;
  ret = gconf_client_set_float(gconf_client, full_key.c_str(), v, &error);

  if (error != NULL)
    {
      ret = false;
    }
  return ret;
}



bool
GConfConfigurator::add_listener(string key_prefix, ConfiguratorListener *listener)
{
  TRACE_ENTER_MSG("GConfConfigurator::add_listener", key_prefix);

  string full_key = gconf_root + key_prefix;
  GError *error = NULL;
  bool ret = true;

  // Add directorie to watch
  gconf_client_add_dir(gconf_client, full_key.c_str(), GCONF_CLIENT_PRELOAD_NONE, &error);
  if (error != NULL)
    {
      ret = false;
    }

  // Add notification callback.
  guint id = 0;
  if (ret)
    {
      id = gconf_client_notify_add(gconf_client,
                                   full_key.c_str(),
                                   &GConfConfigurator::static_key_changed,
                                   (gpointer)this,
                                   NULL,
                                   &error);

      if (error != NULL)
        {
          ret = false;
        }
    }

  // And add callback to Configurator.
  if (ret)
    {
      id2key_map[id] = key_prefix;
      ids_map[make_pair(key_prefix, listener)] = id;
      ret = Configurator::add_listener(key_prefix, listener);
    }

  // FIXME: cleanup.

  TRACE_EXIT();
  return ret;
}


bool
GConfConfigurator::remove_listener(ConfiguratorListener *listener)
{
  return remove_listener("", listener);
}

bool
GConfConfigurator::remove_listener(string remove_key, ConfiguratorListener *listener)
{
  bool ret = false;

  IDMapIter i = ids_map.begin();
  IDMapIter next;
  while (i != ids_map.end())
    {
      pair<string, ConfiguratorListener *> key = i->first;

      string key_prefix = key.first;

      next = i;
      next++;

      if ((key.first == remove_key || remove_key == "") && key.second == listener)
        {
          guint id = i->second;

          gconf_client_notify_remove(gconf_client, id);

          string full_key = gconf_root + key_prefix;
          GError *error = NULL;

          gconf_client_remove_dir(gconf_client, full_key.c_str(), &error);

          ids_map.erase(i);

          ListenerIDsIter i2 = id2key_map.find(id);
          if (i2 != id2key_map.end())
            {
              id2key_map.erase(i2);
            }

          ret = Configurator::remove_listener(key_prefix, listener);
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
  string dir = id2key_map[id];

  string full_key = entry->key;
  TRACE_MSG(full_key);

  if (full_key.substr(0, gconf_root.length()) == gconf_root)
    {
      full_key = full_key.substr(gconf_root.length());
    }

  TRACE_MSG(full_key);
  fire_configurator_event(full_key);

  TRACE_EXIT();
}
