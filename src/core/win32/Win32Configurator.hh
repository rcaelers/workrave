// Win32Configurator.hh
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
// $Id$
//

#ifndef WIN32CONFIGURATOR_HH
#define WIN32CONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include <windows.h>
#include "Configurator.hh"

class ConfigurationListener;

class Win32Configurator :
  public Configurator
{
public:
  Win32Configurator();
  virtual ~Win32Configurator();

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
  string key_win32ify(string key) const;
  string key_add_part(string s, string t) const;
  void key_split(string key, string &parent, string &child) const;
  
  string key_root;
  PHKEY key_root_handle;
};

#endif // WIN32CONFIGURATOR_HH
