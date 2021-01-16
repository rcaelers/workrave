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

#ifndef GSETTINGSCONFIGURATOR_HH
#define GSETTINGSCONFIGURATOR_HH

#include <string>
#include <map>

#include <glib.h>
#include <gio/gio.h>

#include "IConfigBackend.hh"

class GSettingsConfigurator
  : public IConfigBackend
  , public IConfigBackendMonitoring
{
public:
  GSettingsConfigurator();
  ~GSettingsConfigurator() override = default;

  bool load(std::string filename) override;
  bool save(std::string filename) override;
  bool save() override;

  bool remove_key(const std::string &key) override;
  bool get_value(const std::string &key, VariantType type, Variant &value) const override;
  bool set_value(const std::string &key, Variant &value) override;

  void set_listener(workrave::config::IConfiguratorListener *listener) override;
  bool add_listener(const std::string &key_prefix) override;
  bool remove_listener(const std::string &key_prefix) override;

private:
  //! Send changes to.
  workrave::config::IConfiguratorListener *listener{nullptr};

  std::string schema_base;
  std::string path_base;

  using SettingsMap = std::map<std::string, GSettings *>;
  using SettingsIter = SettingsMap::iterator;
  using SettingsCIter = SettingsMap::const_iterator;

  //!
  SettingsMap settings;

  void add_children();
  void key_split(const std::string &key, std::string &parent, std::string &child) const;
  GSettings *get_settings(const std::string &path, std::string &key) const;

  static void on_settings_changed(GSettings *settings, const gchar *key, void *user_data);
};

#endif // GGSETTINGSCONFIGURATOR_HH
