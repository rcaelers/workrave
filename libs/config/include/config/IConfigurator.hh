// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_CONFIG_ICONFIGURATOR_HH
#define WORKRAVE_CONFIG_ICONFIGURATOR_HH

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <optional>

namespace workrave
{
  namespace config
  {
    class IConfiguratorListener;

    enum class ConfigType
    {
      Unknown,
      Boolean,
      Int32,
      Int64,
      Double,
      String,
    };

    using ConfigValue = std::variant<bool, int32_t, int64_t, double, std::string>;

    //! Hints on how to set a configuration value.
    enum ConfigFlags
    {
      //! No special hints.
      CONFIG_FLAG_NONE = 0,

      //! The initial value is set.
      CONFIG_FLAG_INITIAL = 1,

      //! The value must be set immediately, without delay.
      CONFIG_FLAG_IMMEDIATE = 2
    };

    class IConfigurator
    {
    public:
      using Ptr = std::shared_ptr<IConfigurator>;

    public:
      virtual ~IConfigurator() = default;

      virtual void set_delay(const std::string &key, int delay) = 0;

      virtual void heartbeat() = 0;
      virtual bool load(std::string filename) = 0;
      virtual void save() = 0;

      virtual void remove_key(const std::string &key) const = 0;
      virtual void rename_key(const std::string &key, const std::string &new_key) = 0;

      virtual bool has_user_value(const std::string &key) = 0;

      virtual bool get_value(const std::string &key, std::string &out) const = 0;
      virtual bool get_value(const std::string &key, bool &out) const = 0;
      virtual bool get_value(const std::string &key, int32_t &out) const = 0;
      virtual bool get_value(const std::string &key, int64_t &out) const = 0;
      virtual bool get_value(const std::string &key, double &out) const = 0;

      virtual std::optional<ConfigValue> get_value(const std::string &key, ConfigType type) const = 0;

      virtual void get_value_with_default(const std::string &key, std::string &out, const std::string &s) const = 0;
      virtual void get_value_with_default(const std::string &key, bool &out, bool def) const = 0;
      virtual void get_value_with_default(const std::string &key, int32_t &out, int32_t def) const = 0;
      virtual void get_value_with_default(const std::string &key, int64_t &out, int64_t def) const = 0;
      virtual void get_value_with_default(const std::string &key, double &out, double def) const = 0;

      virtual void set_value(const std::string &key,
                             const std::string &v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;
      virtual void set_value(const std::string &key,
                             const char *v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;
      virtual void set_value(const std::string &key,
                             int32_t v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;
      virtual void set_value(const std::string &key,
                             int64_t v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;
      virtual void set_value(const std::string &key,
                             bool v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;
      virtual void set_value(const std::string &key,
                             double v,
                             workrave::config::ConfigFlags flags = workrave::config::CONFIG_FLAG_NONE) = 0;

      virtual bool add_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) = 0;
      virtual bool remove_listener(workrave::config::IConfiguratorListener *listener) = 0;
      virtual bool remove_listener(const std::string &key_prefix, workrave::config::IConfiguratorListener *listener) = 0;
    };
  } // namespace config
} // namespace workrave

#endif // WORKRAVE_CONFIG_ICONFIGURATOR_HH
