// GConfConfigurator.hh
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
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

#ifndef GCONFCONFIGURATOR_HH
#define GCONFCONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <gconf/gconf-client.h>

#include "Configurator.hh"

class ConfigurationListener;

class GConfConfigurator :
  public Configurator
{
public:
  GConfConfigurator();
  virtual ~GConfConfigurator();

  virtual bool load(string filename);
  virtual bool save(string filename);
  virtual bool save();
  virtual bool get_value(string key, string *out) const;
  virtual bool get_value(string key, bool *out) const;
  virtual bool get_value(string key, int *out) const;
  virtual bool get_value(string key, long *out) const;
  virtual bool get_value(string key, double *out) const;
  virtual bool set_value(string key, string v);
  virtual bool set_value(string key, int v);
  virtual bool set_value(string key, long v);
  virtual bool set_value(string key, bool v);
  virtual bool set_value(string key, double v);
  virtual list<string> get_all_dirs(string key) const;
  virtual bool exists_dir(string key) const;
  virtual bool add_listener(string keyPrefix, ConfiguratorListener *listener);
  virtual bool remove_listener(ConfiguratorListener *listener);

  
  
private:
  // void key_changed(guint id, Gnome::Conf::Entry entry);
  
private:
  typedef map<guint, string> ListenerIDs;
  typedef map<guint, string>::iterator ListenerIDsIter;
  
  //! GConf IDs to paths map.
  ListenerIDs listener_ids;
  
  //! GConf thingy
  GConfClient *gconf_client;

  //! GConf Root of workrave
  string gconf_root;
  
  //! Callback.
  static void static_key_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);  

  //!
  void key_changed(guint cnxn_id, GConfEntry *entry);  
};

#endif // GCONFCONFIGURATOR_HH
