// Copyright (C) 2005 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include "IniConfigurator.hh"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ini_parser.hpp>

bool
IniConfigurator::load(std::string filename)
{
  bool ret = false;

  try
    {
      last_filename = filename;
      boost::property_tree::ini_parser::read_ini(filename, pt);
      ret = !pt.empty();
    }
  catch (boost::property_tree::ini_parser_error &e)
    {
      logger->error("failed to load ({})", e.what());
    }

  return ret;
}

void
IniConfigurator::save()
{
  try
    {
      std::ofstream config_file(last_filename.c_str());
      boost::property_tree::ini_parser::write_ini(config_file, pt);
    }
  catch (boost::property_tree::ini_parser_error &e)
    {
      logger->error("failed to save ({})", e.what());
    }
}

bool
IniConfigurator::has_user_value(const std::string &key)
{
  bool ret = true;

  try
    {
      boost::property_tree::ptree::path_type inikey = path(key);
      pt.get_child(inikey);
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to read {} ({})", key, e.what());
      ret = false;
    }

  return ret;
}

void
IniConfigurator::remove_key(const std::string &key)
{
  try
    {
      logger->debug("remove {}", key);
      std::string::size_type pos = key.find('/');
      if (std::string::npos != pos)
        {
          std::string inikey = key.substr(pos + 1);
          std::string section = key.substr(0, pos);
          boost::replace_all(inikey, "/", ".");

          pt.get_child(section).erase(inikey);
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to remove {} ({})", key, e.what());
    }
}

std::optional<ConfigValue>
IniConfigurator::get_value(const std::string &key, ConfigType type) const
{
  try
    {
      boost::property_tree::ptree::path_type inikey = path(key);
      logger->debug("read {} = {}", key, pt.get<std::string>(inikey));

      switch (type)
        {
        case ConfigType::Unknown:
          return pt.get<std::string>(inikey);

        case ConfigType::Int32:
          return pt.get<int32_t>(inikey);

        case ConfigType::Int64:
          return pt.get<int64_t>(inikey);

        case ConfigType::Boolean:
          return pt.get<bool>(inikey);

        case ConfigType::Double:
          return pt.get<double>(inikey);

        case ConfigType::String:
          return pt.get<std::string>(inikey);
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to read {} ({})", key, e.what());
    }
  return {};
}

void
IniConfigurator::set_value(const std::string &key, const ConfigValue &value)
{
  try
    {
      boost::property_tree::ptree::path_type inikey = path(key);

      std::visit(
        [inikey, key, this](auto &&value) {
          using T = std::decay_t<decltype(value)>;

          if constexpr (!std::is_same_v<std::monostate, T>)
            {
              logger->debug("write {} = {}", key, value);
              pt.put(inikey, value);
            }
        },
        value);
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to write {} ({})", key, e.what());
    }
}

boost::property_tree::ptree::path_type
IniConfigurator::path(const std::string &key)
{
  std::string new_key = key;
  bool first = true;
  for (auto &c: new_key)
    {
      if (c == '/')
        {
          if (!first)
            {
              c = '.';
            }
          first = false;
        }
    }

  return boost::property_tree::ptree::path_type(new_key, '/');
}
