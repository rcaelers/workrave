// Configurator.cc --- Configuration Access
//
// Copyright (C) 2002, 2003, 2006, 2007 Rob Caelers <robc@krandor.org>
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
#include <cstdlib>
#include <sstream>

#include "Configurator.hh"
#include "ConfiguratorListener.hh"

#ifdef HAVE_GLIB
#include "GlibIniConfigurator.hh"
#endif
#ifdef HAVE_QT
#include "QtIniConfigurator.hh"
#include "QtNativeConfigurator.hh"
#endif
#ifdef HAVE_GDOME
#include "XMLConfigurator.hh"
#endif
#ifdef HAVE_GCONF
#include "GConfConfigurator.hh"
#endif
#ifdef HAVE_REGISTRY
#include "W32Configurator.hh"
#endif


//! Creates a configurator of the specified type.
Configurator *
Configurator::create(Format fmt)
{
  Configurator *c =  NULL;

#ifdef HAVE_GDOME  
  if (fmt == FormatXml)
    {
      c = new XMLConfigurator();
    }
  else
#endif
    
#ifdef HAVE_GCONF
  if (fmt == FormatNative)
    {
      c = new GConfConfigurator();
    }
  else
#endif
    
#ifdef HAVE_REGISTRY
  if (fmt == FormatNative)
    {
      c = new W32Configurator();
    }
  else
#endif    

#ifdef HAVE_QT
  if (fmt == FormatNative)
    {
      c = new QtNativeConfigurator();
    }
  else
#endif    

  if (fmt == FormatIni)
    {
#ifdef HAVE_GLIB
      c = new GlibIniConfigurator();
#elif defined(HAVE_QT)
      c = new QtIniConfigurator();
#else
#error Not ported
#endif      
    }
  else
    {
      exit(1);
    }
  return c;
}


// Constructs a new configurator.
Configurator::Configurator()
{
  head = new keylist;
  mutex = new Mutex;
}


// Destructs the configurator.
Configurator::~Configurator()
{
  delete head;
  delete mutex;
}


//! Add the specified configuration change listener.
/*!
 *  \param listener listener to add.
 *
 *  \retval true listener successfully added.
 *  \retval false listener already added.
 */
bool
Configurator::add_listener(string key_prefix, ConfiguratorListener *listener)
{
  bool ret = true;

  strip_leading_slash(key_prefix);
  
  ListenerIter i = listeners.begin();
  while (ret && i != listeners.end())
    {
      if (key_prefix == i->first && listener == i->second)
        {
          // Already added. Skip
          ret = false;
        }
      
      i++;
    }
  
  if (ret)
    {
      // not found -> add
      listeners.push_back(make_pair(key_prefix, listener));
    }

  return ret;
}


//! Removes the specified configuration change listener.
/*!
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
      if (listener == i->second)
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


//! Removes the specified configuration change listener.
/*!
 *  \param listener listener to remove.
 *  \param listener key of listener to remove.
 *
 *  \retval true listener successfully removed.
 *  \retval false listener not found.
 */
bool
Configurator::remove_listener(string key, ConfiguratorListener *listener)
{
  bool ret = false;

  ListenerIter i = listeners.begin();
  while (i != listeners.end())
    {
      if (i->first == key && i->second == listener)
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



//! Finds the key of the specified configuration change listener.
/*!
 *  \param listener listener to find the key of.
 *  \param key returned key.
 *
 *  \retval true listener successfully found.
 *  \retval false listener not found.
 */
bool
Configurator::find_listener(ConfiguratorListener *listener, string &key) const
{
  bool ret = false;

  ListenerCIter i = listeners.begin();
  while (i != listeners.end())
    {
      if (listener == i->second)
        {
          key = i->first;
          ret = true;
          break;
        }
      i++;
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


//! Removes the leading '/'.
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


//! Removes the trailing '/'.
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


//! Adds add trailing '/' if it isn't there yet.
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

void
Configurator::get_value_default(string key, int *out, const int def) const
{
  bool b = get_value(key, out);
  if (! b)
    {
      *out = def;
      b = true;
    }
}



void
Configurator::get_value_default(string key, bool *out, const bool def) const
{
  bool b = get_value(key, out);
  if (! b)
    {
      *out = def;
    }
}

void
Configurator::get_value_default(string key, string *out,
                                const string def) const
{
  bool b = get_value(key, out);
  if (! b)
    {
      *out = def;
    }
}


void
Configurator::get_value_default(string key, long *out,
                                const long def) const
{
  bool b = get_value(key, out);
  if (! b)
    {
      *out = def;
    }
}


void
Configurator::get_value_default(string key, double *out,
                                const double def) const
{
  bool b = get_value(key, out);
  if (! b)
    {
      *out = def;
    }
}


// func overloads bool/int/long/double copied from W32Config:
bool 
Configurator::get_value_on_quit( string key, bool *out ) const
{
  long l;
  bool rc = get_value_on_quit( key, &l );
  if ( rc )
    {
      *out = ( bool ) l;
    }
  return rc;
}

bool 
Configurator::get_value_on_quit( string key, int *out ) const
{
  long l;
  bool rc = get_value_on_quit( key, &l );
  if ( rc )
    {
      *out = ( int ) l;
    }
  return rc;
}

bool 
Configurator::get_value_on_quit( string key, long *out ) const
{
  string s;
  bool rc = get_value_on_quit( key, &s );
  if ( rc )
    {
      int f = sscanf( s.c_str(), "%ld", out );
      rc = ( f == 1 );
    }
  return rc;
}

bool 
Configurator::get_value_on_quit( string key, double *out ) const
{
  string s;
  bool rc = get_value_on_quit( key, &s );
  if ( rc )
    {
      int f = sscanf( s.c_str(), "%lf", out );
      rc = ( f == 1 );
    }
  return rc;
}

bool 
Configurator::get_value_on_quit( string key, string *out ) const
{
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->lock();
#endif
  
  keylist *temp = head;
  
  while( 1 )
    {
      if( temp->key == key )
      // Key found, assign value to out
	    {
	      *out = temp->v;
          return true;
        }
      
      // Step through the list:
      if( temp->next )
        temp = temp->next;
      else
        return false;
    }
  
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->unlock();
#endif
}


// func overloads bool/int/long/double copied from W32Config:
void 
Configurator::set_value_on_quit( string key, bool v )
{
  char buf[32];
  sprintf( buf, "%d", v ? 1 : 0 );
  set_value_on_quit( key, string( buf ) );
}

void 
Configurator::set_value_on_quit( string key, int v )
{
  char buf[32];
  sprintf( buf, "%d", v );
  set_value_on_quit( key, string( buf ) );
}

void 
Configurator::set_value_on_quit( string key, long v )
{
  char buf[32];
  sprintf( buf, "%ld", v );
  set_value_on_quit( key, string( buf ) );
}

void 
Configurator::set_value_on_quit( string key, double v )
{
  char buf[32];
  sprintf( buf, "%f", v );
  set_value_on_quit( key, string( buf ) );
}

void 
Configurator::set_value_on_quit( string key, string v )
{
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->lock();
#endif
  
  keylist *temp = head;
  
  while( 1 )
    {
      if( temp->key == key )
      // If the key is already in the list, update its value.
        {
          temp->v = v;
          return;
        }
      
      // Step through the list:
      if( temp->next )
        temp = temp->next;
      else
        break;
    }
  
  if( temp->key != "" )
    {
      temp->next = new keylist;
      temp = temp->next;
    }
  
  temp->key = key;
  temp->v = v;
  
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->unlock();
#endif
}


void 
Configurator::set_values_now_we_are_quitting()
// This function should be called only from the Core destructor!
// Do not call from Configurator destructor, because the virtual 
// functions will have already been destroyed.
{
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->lock();
#endif
  
  keylist *temp = head;
  
  while( 1 )
    {
      if( temp->key != "" )
        set_value( temp->key, temp->v );
      
      // Step through the list:
      if( temp->next )
        temp = temp->next;
      else
        break;
    }
  
/*
  Maybe this function is called, and the program 
  isn't terminating... so cleanup.
*/
  keylist *temp2 = head->next;
  // delete all members except for the head:
  while( temp = temp2 )
    {
      temp2 = temp->next;
      delete temp;
    }
  
  head->key = "";
  head->v = "";
  head->next = NULL;
  
#if defined(HAVE_QT) || defined(WIN32) || defined(HAVE_UNIX)
  mutex->unlock();
#endif
}
