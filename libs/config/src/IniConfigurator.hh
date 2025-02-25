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

#ifndef INICONFIGURATOR_HH
#define INICONFIGURATOR_HH

#include <string>
#include <boost/property_tree/ptree.hpp>

#include "IConfigBackend.hh"
#include "utils/Logging.hh"

class IniConfigurator : public virtual IConfigBackend
{
public:
  IniConfigurator() = default;
  ~IniConfigurator() override = default;

  bool load(std::string filename) override;
  void save() override;

  void remove_key(const std::string &key) override;
  bool has_user_value(const std::string &key) override;
  std::optional<ConfigValue> get_value(const std::string &key, ConfigType type) const override;
  void set_value(const std::string &key, const ConfigValue &value) override;

private:
  static boost::property_tree::ptree::path_type path(const std::string &key);

private:
  boost::property_tree::ptree pt;
  std::string last_filename;
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("config:ini")};
};

#endif // INICONFIGURATOR_HH
