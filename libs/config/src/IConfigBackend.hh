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

#ifndef ICONFIGBACKEND_HH
#define ICONFIGBACKEND_HH

#include <string>
#include <iostream>

#include "config/IConfigurator.hh"

using ConfigValue = workrave::config::ConfigValue;
using ConfigType = workrave::config::ConfigType;

constexpr ConfigType
ConfigValueToType(ConfigValue &value)
{
  return std::visit(
    [](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (std::is_same_v<bool, T>)
        {
          return ConfigType::Boolean;
        }
      else if constexpr (std::is_same_v<int64_t, T>)
        {
          return ConfigType::Int64;
        }
      else if constexpr (std::is_same_v<int32_t, T>)
        {
          return ConfigType::Int32;
        }
      else if constexpr (std::is_same_v<double, T>)
        {
          return ConfigType::Double;
        }
      else if constexpr (std::is_same_v<std::string, T>)
        {
          return ConfigType::String;
        }
    },
    value);
}

inline std::ostream &
operator<<(std::ostream &os, const ConfigValue &value)
{
  std::visit([&os](auto &&arg) { os << arg; }, value);
  return os;
}

class IConfigBackend
{
public:
  virtual ~IConfigBackend() = default;

  virtual bool load(std::string filename) = 0;
  virtual void save() = 0;

  virtual void remove_key(const std::string &key) = 0;
  virtual bool has_user_value(const std::string &key) = 0;
  virtual std::optional<ConfigValue> get_value(const std::string &key, ConfigType type) const = 0;
  virtual void set_value(const std::string &key, const ConfigValue &value) = 0;
};

class IConfigBackendMonitoring
{
public:
  virtual ~IConfigBackendMonitoring() = default;

  virtual void set_listener(workrave::config::IConfiguratorListener *listener) = 0;
  virtual bool add_listener(const std::string &key_prefix) = 0;
  virtual bool remove_listener(const std::string &key_prefix) = 0;
};

#endif // ICONFIGBACKEND_HH
