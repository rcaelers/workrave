// Copyright (C) 2008 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef MACOSCONFIGURATOR_HH
#define MACOSCONFIGURATOR_HH

#include <string>

#include "utils/Logging.hh"

#include "IConfigBackend.hh"

class MacOSConfigurator : public virtual IConfigBackend
{
public:
  MacOSConfigurator() = default;
  ~MacOSConfigurator() override = default;

  bool load(std::string filename) override;
  void save() override;

  void remove_key(const std::string &key) override;
  bool has_user_value(const std::string &key) override;
  std::optional<ConfigValue> get_value(const std::string &key, ConfigType type) const override;
  void set_value(const std::string &key, const ConfigValue &value) override;

private:
  std::shared_ptr<spdlog::logger> logger{workrave::utils::Logging::create("config:macos")};
};

#endif // MACOSCONFIGURATOR_HH
