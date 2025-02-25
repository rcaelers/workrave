// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
//
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

#ifndef CONFIGURATOR_HH
#define CONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include "config/IConfigurator.hh"
#include "config/IConfiguratorListener.hh"
#include "IConfigBackend.hh"

#include "utils/Logging.hh"

class Configurator
  : public workrave::config::IConfigurator
  , public workrave::config::IConfiguratorListener
{
public:
  explicit Configurator(IConfigBackend *backend);
  ~Configurator() override;

  void heartbeat() override;

  void set_delay(const std::string &key, int delay) override;

  bool load(std::string filename) override;
  void save() override;

  bool has_user_value(const std::string &key) override;

  void remove_key(const std::string &key) const override;
  void rename_key(const std::string &key, const std::string &new_key) override;

  bool get_value(const std::string &key, std::string &out) const override;
  bool get_value(const std::string &key, bool &out) const override;
  bool get_value(const std::string &key, int32_t &out) const override;
  bool get_value(const std::string &key, int64_t &out) const override;
  bool get_value(const std::string &key, double &out) const override;
  std::optional<ConfigValue> get_value(const std::string &key, workrave::config::ConfigType type) const override;

  void get_value_with_default(const std::string &key, std::string &out, const std::string &def) const override;
  void get_value_with_default(const std::string &key, bool &out, bool def) const override;
  void get_value_with_default(const std::string &key, int32_t &out, int32_t def) const override;
  void get_value_with_default(const std::string &key, int64_t &out, int64_t def) const override;
  void get_value_with_default(const std::string &key, double &out, double def) const override;

  void set_value(const std::string &key,
                 const std::string &v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  void set_value(const std::string &key,
                 const char *v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  void set_value(const std::string &key,
                 int32_t v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  void set_value(const std::string &key,
                 int64_t v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  void set_value(const std::string &key,
                 bool v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  void set_value(const std::string &key,
                 double v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;

  bool add_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) override;
  bool remove_listener(workrave::config::IConfiguratorListener *listener) override;
  bool remove_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) override;

private:
  struct DelayedConfig
  {
    std::string key;
    ConfigValue value;
    int64_t until;
  };

private:
  bool set_value(const std::string &key,
                 ConfigValue &value,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE);

  static std::string trim_key(const std::string &key);

  void fire_configurator_event(const std::string &key);
  void config_changed_notify(const std::string &key) override;

private:
  std::map<std::string, int> delays;
  std::map<std::string, DelayedConfig> delayed_config;
  std::list<std::pair<std::string, workrave::config::IConfiguratorListener *>> listeners;
  IConfigBackend *backend{nullptr};
  int64_t auto_save_time{0};
  std::string last_filename;
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("config")};
};

#endif // CONFIGURATOR_HH
