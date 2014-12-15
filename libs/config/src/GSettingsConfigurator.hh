// GConfConfigurator.hh
//
// Copyright (C) 2011 Caelers <robc@krandor.nl>
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

#ifndef GCONFCONFIGURATOR_HH
#define GCONFCONFIGURATOR_HH

#include <string>
#include <map>

#include <glib.h>
#include <gio/gio.h>

#include "IConfigBackend.hh"

class GSettingsConfigurator :
  public IConfigBackend, public IConfigBackendMonitoring
{
public:
  GSettingsConfigurator();
  virtual ~GSettingsConfigurator();

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);
  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const;
  virtual bool set_value(const std::string &key, Variant &value);

  virtual void set_listener(IConfiguratorListener *listener);
  virtual bool add_listener(const std::string &key_prefix);
  virtual bool remove_listener(const std::string &key_prefix);

private:
  //! Send changes to.
  IConfiguratorListener *listener;

  std::string schema_base;
  std::string path_base;
  
  typedef std::map<std::string, GSettings *> SettingsMap;
  typedef SettingsMap::iterator SettingsIter;
  typedef SettingsMap::const_iterator SettingsCIter;

  //! 
  SettingsMap settings;
  
  void add_children();
  void key_split(const string &key, string &parent, std::string &child) const;
  GSettings *get_settings(const std::string &path, std::string &key) const;
  
  static void on_settings_changed(GSettings *settings, const gchar *key, void *user_data);
};


#endif // GCONFCONFIGURATOR_HH
