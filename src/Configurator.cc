// Configurator.cc --- Configuration Access
//
// Copyright (C) 2002 Rob Caelers <robc@krandor.org>
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

#include "Configurator.hh"
#include "ConfiguratorListener.hh"

#ifdef HAVE_GDOME
#include "XMLConfigurator.hh"
#endif
#ifdef HAVE_GCONF
#include "GConfConfigurator.hh"
#endif
#ifdef HAVE_REGISTRY
#include "Win32Configurator.hh"
#endif


Configurator *
Configurator::create(string type)
{
  Configurator *c =  NULL;

#ifdef HAVE_GDOME  
  if (type == "xml")
    {
      c = new XMLConfigurator();
    }
  else
#endif
    
#ifdef HAVE_GCONF
  if (type == "gconf")
    {
      c = new GConfConfigurator();
    }
  else
#endif
    
#ifdef HAVE_REGISTRY
  if (type == "w32")
    {
      c = new Win32Configurator();
    }
  else
#endif    
    {
      exit(1);
    }
  return c;
}

Configurator::Configurator()
{
}

Configurator::~Configurator()
{
}

/*!
 *  \param listener listener to add.
 *
 *  \retval true listener successfully added.
 *  \retval false listener already added.
 */
bool
Configurator::add_listener(string keyPrefix, ConfiguratorListener *listener)
{
  bool ret = true;

  strip_leading_slash(keyPrefix);
  
  ListenerIter i = listeners.begin();
  while (ret && i != listeners.end())
    {
      string p = i->first;
      ConfiguratorListener *l = i->second;

      if (keyPrefix == p && listener == l)
        {
          // Already added. Skip
          ret = false;
        }
      
      i++;
    }
  
  if (ret)
    {
      // not found -> add
      listeners.push_back(make_pair(keyPrefix, listener));
    }

  return ret;
}


//! Remove the specified activity listener.
/*!
 *
 *  \param listener listener to remove.
 *
 *  \retval true listener successfully removed.
 *  \retval false listener not found.
 */
bool
Configurator::remove_listener(ConfiguratorListener *listener)
{
  bool ret = false;

  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      ConfiguratorListener *l = i->second;

      if (listener == l)
        {
          // Found. Remove
          i = listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }

  return ret;
}


//! Fire a configuration changed event.
void
Configurator::fire_configurator_event(string key)
{
  TRACE_ENTER_MSG("Configurator::fire_configurator_event", key);
  strip_leading_slash(key);
  
  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      string prefix = i->first;

      if (key.substr(0, prefix.length()) == prefix)
        {
          ConfiguratorListener *l = i->second;
          if (l != NULL)
            {
              l->config_changed_notify(key);
            }
        }
      i++;
    }

  TRACE_EXIT();
}


//! Removes the leading '/'
void
Configurator::strip_leading_slash(string &key) const
{
  int len = key.length();
  if (len > 1)
    {
      if (key[0] == '/')
        {
          key = key.substr(1, len - 1);
        }
    }
}


//! Removes the trailing '/'
void
Configurator::strip_trailing_slash(string &key) const
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


//! Add add trailing '/' if it isn't there yet.
void
Configurator::add_trailing_slash(string &key) const
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
