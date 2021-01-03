// W32Configurator.hh
//
// Copyright (C) 2002, 2006, 2007, 2012 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef W32CONFIGURATOR_HH
#define W32CONFIGURATOR_HH

#include <string>
#include <list>

#include <windows.h>
#include "IConfigBackend.hh"
#include "ConfigBackendAdapter.hh"

class W32Configurator :
  public virtual IConfigBackend, public virtual ConfigBackendAdapter
{
public:
  W32Configurator();
  virtual ~W32Configurator();

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);

  virtual bool get_config_value(const std::string &key, std::string &out) const;
  virtual bool get_config_value(const std::string &key, bool &out) const;
  virtual bool get_config_value(const std::string &key, int &out) const;
  virtual bool get_config_value(const std::string &key, long &out) const;
  virtual bool get_config_value(const std::string &key, double &out) const;

  virtual bool set_config_value(const std::string &key, std::string v);
  virtual bool set_config_value(const std::string &key, int v);
  virtual bool set_config_value(const std::string &key, long v);
  virtual bool set_config_value(const std::string &key, bool v);
  virtual bool set_config_value(const std::string &key, double v);

private:
  std::string key_win32ify(const std::string &key) const;
  std::string key_add_part(std::string s, std::string t) const;
  void key_split(const std::string &key, std::string &parent, std::string &child) const;

  void strip_trailing_slash(std::string &key) const;
  void add_trailing_slash(std::string &key) const;

  std::string key_root;
};

#endif // W32CONFIGURATOR_HH
