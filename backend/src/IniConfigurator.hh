// IniConfigurator.hh
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

#ifndef INICONFIGURATOR_HH
#define INICONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <glib.h>

#include "Configurator.hh"

class ConfigurationListener;

class IniConfigurator :
  public Configurator
{
public:
  IniConfigurator();
  virtual ~IniConfigurator();

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

private:
  void split_key(const string key, string &group, string &out_key) const;
  string key_inify(string key) const;
  
private:
  GKeyFile *config;

  string last_filename;
};

#endif // INICONFIGURATOR_HH
