// Copyright (C) 2005, 2006, 2007, 2008, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include <cstring>
#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>

#include "debug.hh"

using namespace std;

bool
XmlConfigurator::load(string filename)
{
  TRACE_ENTER_MSG("XmlConfigurator::loada", filename);
  bool ret = false;

  try
    {
      boost::property_tree::xml_parser::read_xml(filename, pt);
      ret = !pt.empty();
      last_filename = filename;
    }
  catch (boost::property_tree::xml_parser_error &)
    {
    }

  TRACE_EXIT();
  return ret;
}

bool
XmlConfigurator::save(string filename)
{
  TRACE_ENTER_MSG("XmlConfigurator::save", filename);
  bool ret = false;

  try
    {
      ofstream config_file(filename.c_str());
      boost::property_tree::xml_parser::write_xml(config_file, pt);
      ret = true;
    }
  catch (boost::property_tree::xml_parser_error &)
    {
    }

  TRACE_EXIT();
  return ret;
}

bool
XmlConfigurator::save()
{
  return save(last_filename);
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
      ret = false;
    }

  return ret;
}

bool
XmlConfigurator::remove_key(const std::string &key)
{
  bool ret = true;

  TRACE_ENTER_MSG("XmlConfigurator::remove_key", key);

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

  TRACE_EXIT();
  return ret;
}

bool
XmlConfigurator::get_value(const std::string &key, VariantType type, Variant &out) const
{
  TRACE_ENTER_MSG("XmlConfigurator::get_value", key);

  bool ret = true;
  string xmlpath = path(key);

  out.type = type;

  try
    {
      switch (type)
        {
        case VARIANT_TYPE_INT:
          out.int_value = pt.get<int>(xmlpath);
          break;

        case VARIANT_TYPE_BOOL:
          out.bool_value = pt.get<bool>(xmlpath);
          break;

        case VARIANT_TYPE_DOUBLE:
          out.double_value = pt.get<double>(xmlpath);
          break;

        case VARIANT_TYPE_NONE:
          out.type = VARIANT_TYPE_STRING;
          out.string_value = pt.get<string>(xmlpath);
          break;

        case VARIANT_TYPE_STRING:
          out.string_value = pt.get<string>(xmlpath);
          break;
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      ret = false;
    }

  TRACE_RETURN(ret);
  return ret;
}

bool
XmlConfigurator::set_value(const std::string &key, Variant &value)
{
  bool ret = true;

  string xmlpath = path(key);

  try
    {
      switch (value.type)
        {
        case VARIANT_TYPE_INT:
          pt.put(xmlpath, value.int_value);
          break;

        case VARIANT_TYPE_BOOL:
          pt.put(xmlpath, value.bool_value);
          break;

        case VARIANT_TYPE_DOUBLE:
          pt.put(xmlpath, value.double_value);
          break;

        case VARIANT_TYPE_NONE:
        case VARIANT_TYPE_STRING:
          pt.put(xmlpath, value.string_value);
          break;
        }
    }
  catch (boost::property_tree::ptree_error &)
    {
      ret = false;
    }

  return ret;
}

string
XmlConfigurator::path(const string &key) const
{
  vector<string> parts;
  boost::split(parts, key, boost::is_any_of("/"));

  string attr = parts.back();
  parts.pop_back();

  return string("workrave.") + boost::algorithm::join(parts, ".") + ".<xmlattr>." + attr;
}
