// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "IConfigurator.hh"
#include "IConfiguratorListener.hh"
#include "IConfigBackend.hh"
#include "Variant.hh"

class Configurator
  : public workrave::config::IConfigurator
  , public workrave::config::IConfiguratorListener
{
public:
  explicit Configurator(IConfigBackend *backend);
  ~Configurator() override;

  void heartbeat() override;

  // IConfigurator
  void set_delay(const std::string &name, int delay) override;

  bool load(std::string filename) override;
  bool save(std::string filename) override;
  bool save() override;

  bool remove_key(const std::string &key) const override;
  bool rename_key(const std::string &key, const std::string &new_key) override;

  bool get_value(const std::string &key, std::string &out) const override;
  bool get_value(const std::string &key, bool &out) const override;
  bool get_value(const std::string &key, int &out) const override;
  bool get_value(const std::string &key, double &out) const override;

  void get_value_with_default(const std::string &key, std::string &out, std::string s) const override;
  void get_value_with_default(const std::string &key, bool &out, const bool def) const override;
  void get_value_with_default(const std::string &key, int &out, const int def) const override;
  void get_value_with_default(const std::string &key, double &out, const double def) const override;

  bool set_value(const std::string &key,
                 const std::string &v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  bool set_value(const std::string &key,
                 const char *v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  bool set_value(const std::string &key, int v, workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  bool set_value(const std::string &key, bool v, workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;
  bool set_value(const std::string &key,
                 double v,
                 workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) override;

  bool get_typed_value(const std::string &key, std::string &t) const override;
  bool set_typed_value(const std::string &key, const std::string &t) override;

  bool add_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) override;
  bool remove_listener(workrave::config::IConfiguratorListener *listener) override;
  bool remove_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) override;
  bool find_listener(workrave::config::IConfiguratorListener *listener, std::string &key) const override;

private:
  using Listeners = std::list<std::pair<std::string, workrave::config::IConfiguratorListener *>>;

  //! Configuration change listeners.
  Listeners listeners;

private:
  struct DelayedConfig
  {
    std::string key;
    Variant value;
    int64_t until;
  };

  struct Setting
  {
    std::string key;
    int delay;
  };

  using DelayedList = std::map<std::string, DelayedConfig>;
  using Settings = std::map<std::string, Setting>;

private:
  bool find_setting(const std::string &name, Setting &setting) const;

  bool set_value(const std::string &key, Variant &value, workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE);
  bool get_value(const std::string &key, VariantType type, Variant &value) const;

  void fire_configurator_event(const std::string &key);
  void strip_leading_slash(std::string &key) const;
  void strip_trailing_slash(std::string &key) const;
  void add_trailing_slash(std::string &key) const;

  void config_changed_notify(const std::string &key) override;

private:
  //! Registered settings.
  Settings settings;

  //! Delayed settings
  DelayedList delayed_config;

  //! The backend in use.
  IConfigBackend *backend{nullptr};

  //! Next auto save time.
  int64_t auto_save_time{0};
};

#endif // CONFIGURATOR_HH
