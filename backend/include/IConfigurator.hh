// Configurator.hh 
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006 Rob Caelers <robc@krandor.org>
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
// $Id$
//

#ifndef ICONFIGURATOR_HH
#define ICONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

using namespace std;

class ConfiguratorListener;

class IConfigurator
{
public:
  virtual ~IConfigurator() {}
  
  //! Loads the specified file.
  /*!
   *  \param filename file to load.
   *
   *  \retval true load succeeded.
   *  \retval false load failed.
   */
  virtual bool load(string filename) = 0;

  //! Saves the configuration to the specified file.
  /*!
   *  \param filename file to save.
   *
   *  \retval true save succeeded.
   *  \retval false save failed.
   */
  virtual bool save(string filename) = 0;

  //! Saves the configuration.
  /*!
   *  \retval true save succeeded.
   *  \retval false save failed.
   */
  virtual bool save() = 0;

  //! Returns the value of the specified attribute
  /*!
   *  \retval true value successfully returned.
   *  \retval false attribute not found.
   */
  virtual bool get_value(string key, string *out) const = 0;
  virtual void get_value_default(string key, string *out, string s) const = 0;

  //! Returns the value of the specified attribute
  /*!
   *  \retval true value successfully returned.
   *  \retval false attribute not found.
   */
  virtual bool get_value(string key, bool *out) const = 0;
  virtual void get_value_default(string key, bool *out, const bool def)
    const = 0;

  //! Returns the value of the specified attribute
  /*!
   *  \retval true value successfully returned.
   *  \retval false attribute not found.
   */
  virtual bool get_value(string key, int *out) const = 0;
  virtual void get_value_default(string key, int *out, const int def)
    const = 0;

  //! Returns the value of the specified attribute
  /*!
   *  \retval true value successfully returned.
   *  \retval false attribute not found.
   */
  virtual bool get_value(string key, long *out) const = 0;
  virtual void get_value_default(string key, long *out, const long def)
    const = 0;

  //! Returns the value of the specified attribute
  /*!
   *  \retval true value successfully returned.
   *  \retval false attribute not found.
   */
  virtual bool get_value(string key, double *out) const = 0;
  virtual void get_value_default(string key, double *out, const double def)
    const = 0;

  //! Sets the value of the specified attribute.
  /*!
   *  \retval true attribute successfully set.
   *  \retval false attribute could not be set..
   */
  virtual bool set_value(string key, string v) = 0;

  //! Sets the value of the specified attribute.
  /*!
   *  \retval true attribute successfully set.
   *  \retval false attribute could not be set..
   */
  virtual bool set_value(string key, int v) = 0;

  //! Sets the value of the specified attribute.
  /*!
   *  \retval true attribute successfully set.
   *  \retval false attribute could not be set..
   */
  virtual bool set_value(string key, long v) = 0;

  //! Sets the value of the specified attribute.
  /*!
   *  \retval true attribute successfully set.
   *  \retval false attribute could not be set..
   */
  virtual bool set_value(string key, bool v) = 0;

  //! Sets the value of the specified attribute.
  /*!
   *  \retval true attribute successfully set.
   *  \retval false attribute could not be set..
   */
  virtual bool set_value(string key, double v) = 0;

  //! Adds the specified configuration change listener.
  /*!
   *  \param key_prefix configuration path to monitor.
   *  \param listener listener to add.
   *
   *  \retval true listener successfully added.
   *  \retval false listener already added.
   */
  virtual bool add_listener(string key_prefix, ConfiguratorListener *listener) = 0;

  //! Removes the specified configuration change listener.
  /*!
   *  \param listener listener to stop monitoring.
   *
   *  \retval true listener successfully removed.
   *  \retval false listener not found.
   */
  virtual bool remove_listener(ConfiguratorListener *listener) = 0;

  //! Removes the specified configuration change listener.
  /*!
   *  \param key_prefix configuration path to monitor.
   *  \param listener listener to stop monitoring.
   *
   *  \retval true listener successfully removed.
   *  \retval false listener not found.
   */
  virtual bool remove_listener(string key_prefix, ConfiguratorListener *listener) = 0;
  
  //! Finds the key monitored by the the specified configuration change listener.
  /*!
   *  \param listener listener to find the key of.
   *
   */
  virtual bool find_listener(ConfiguratorListener *listener, string &key) const = 0;
  
};

#endif // ICONFIGURATOR_HH
