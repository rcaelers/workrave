// Copyright (C) 2011-2021 Caelers <robc@krandor.nl>
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

#include "utils/Logging.hh"

#include "IConfigBackend.hh"

class GSettingsConfigurator
  : public IConfigBackend
  , public IConfigBackendMonitoring
{
public:
  GSettingsConfigurator();
  ~GSettingsConfigurator() override;

  bool load(std::string filename) override;
  void save() override;

  void remove_key(const std::string &key) override;
  bool has_user_value(const std::string &key) override;
  std::optional<ConfigValue> get_value(const std::string &key, ConfigType type) const override;
  void set_value(const std::string &key, const ConfigValue &value) override;

  void set_listener(workrave::config::IConfiguratorListener *listener) override;
  bool add_listener(const std::string &key_prefix) override;
  bool remove_listener(const std::string &key_prefix) override;

private:
  void add_children();
  static void key_split(const std::string &key, std::string &path, std::string &subkey);
  GSettings *get_settings(const std::string &key, std::string &subkey) const;

  static void on_settings_changed(GSettings *settings, const gchar *key, void *user_data);

private:
  std::string schema_base{"org.workrave"};
  std::string path_base{"/org/workrave/"};

  workrave::config::IConfiguratorListener *listener{nullptr};
  std::map<std::string, GSettings *> settings;
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("config:gsettings")};
};

#endif // GGSETTINGSCONFIGURATOR_HH
