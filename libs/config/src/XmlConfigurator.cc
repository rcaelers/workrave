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

#include "XmlConfigurator.hh"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "IConfigBackend.hh"

bool
XmlConfigurator::load(std::string filename)
{
  bool ret = false;

  try
    {
      last_filename = filename;
      boost::property_tree::xml_parser::read_xml(filename, pt);
      ret = !pt.empty();
    }
  catch (boost::property_tree::xml_parser_error &e)
    {
      logger->error("failed to load ({})", e.what());
    }
  return ret;
}

void
XmlConfigurator::save()
{
  try
    {
      std::ofstream config_file(last_filename.c_str());
      boost::property_tree::xml_parser::write_xml(config_file, pt);
    }
  catch (boost::property_tree::xml_parser_error &e)
    {
      logger->error("failed to save ({})", e.what());
    }
}

bool
XmlConfigurator::has_user_value(const std::string &key)
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
XmlConfigurator::remove_key(const std::string &key)
{
  try
    {
      std::vector<std::string> parts;
      std::string p = path(key);
      boost::split(parts, p, boost::is_any_of("."));

      auto *node = &pt;
      for (const auto &part: parts)
        {
          auto it = node->find(part);
          if (it != node->not_found())
            {
              if (&part == &parts.back())
                {
                  node->erase(node->to_iterator(it));
                }
              else
                {
                  node = &it->second;
                }
            }
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to read {} ({})", key, e.what());
    }
}

std::optional<ConfigValue>
XmlConfigurator::get_value(const std::string &key, ConfigType type) const
{
  try
    {
      logger->debug("read {} = {}", key, pt.get<std::string>(path(key)));
      switch (type)
        {
        case ConfigType::Unknown:
          return pt.get<std::string>(path(key));

        case ConfigType::Int32:
          return pt.get<int32_t>(path(key));

        case ConfigType::Int64:
          return pt.get<int64_t>(path(key));

        case ConfigType::Boolean:
          return pt.get<bool>(path(key));

        case ConfigType::Double:
          return pt.get<double>(path(key));

        case ConfigType::String:
          return pt.get<std::string>(path(key));
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->error("failed to write {} ({})", key, e.what());
    }
  return {};
}

void
XmlConfigurator::set_value(const std::string &key, const ConfigValue &value)
{
  try
    {
      std::visit(
        [key, this](auto &&value) {
          using T = std::decay_t<decltype(value)>;

          if constexpr (!std::is_same_v<std::monostate, T>)
            {
              logger->debug("write {} = {}", key, value);
              pt.put(path(key), value);
            }
        },
        value);
    }
  catch (boost::property_tree::ptree_error &e)
    {
      logger->debug("failed to write {} ({})", key, e.what());
    }
}

std::string
XmlConfigurator::path(const std::string &key)
{
  std::vector<std::string> parts;
  boost::split(parts, key, boost::is_any_of("/"));

  std::string attr = parts.back();
  parts.pop_back();

  return std::string("workrave.") + boost::algorithm::join(parts, ".") + ".<xmlattr>." + attr;
}
