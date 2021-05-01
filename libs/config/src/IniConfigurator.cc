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

#ifdef PLATFORM_OS_MACOS
#  include "MacOSHelpers.hh"
#endif

#include "IniConfigurator.hh"

#include <iostream>
#include <fstream>

#include "debug.hh"

using namespace std;

bool
IniConfigurator::load(string filename)
{
  TRACE_ENTER_MSG("IniConfigurator::load", filename);
  bool ret = false;

  try
    {
      boost::property_tree::ini_parser::read_ini(filename, pt);
      ret = !pt.empty();
      last_filename = filename;
    }
  catch (boost::property_tree::ini_parser_error &)
    {
    }

  TRACE_EXIT();
  return ret;
}

bool
IniConfigurator::save(string filename)
{
  TRACE_ENTER_MSG("IniConfigurator::save", filename);
  bool ret = false;

  try
    {
      ofstream config_file(filename.c_str());
      boost::property_tree::ini_parser::write_ini(config_file, pt);
      ret = true;
    }
  catch (boost::property_tree::ini_parser_error &)
    {
    }

  TRACE_EXIT();
  return ret;
}

bool
IniConfigurator::save()
{
  return save(last_filename);
}

bool
IniConfigurator::remove_key(const std::string &key)
{
  bool ret = true;

  TRACE_ENTER_MSG("IniConfigurator::remove_key", key);
  boost::property_tree::ptree::path_type inikey = path(key);

  TRACE_EXIT();
  return ret;
}

bool
IniConfigurator::get_value(const std::string &key, VariantType type, Variant &out) const
{
  TRACE_ENTER_MSG("IniConfigurator::get_value", key);

  bool ret = true;
  boost::property_tree::ptree::path_type inikey = path(key);

  out.type = type;

  try
    {
      switch (type)
        {
        case VARIANT_TYPE_INT:
          TRACE_MSG("value i = " << out.int_value);
          out.int_value = pt.get<int>(inikey);
          break;

        case VARIANT_TYPE_BOOL:
          TRACE_MSG("value b = " << out.bool_value);
          out.bool_value = pt.get<bool>(inikey);
          break;

        case VARIANT_TYPE_DOUBLE:
          TRACE_MSG("value d = " << out.double_value);
          out.double_value = pt.get<double>(inikey);
          break;

        case VARIANT_TYPE_NONE:
          out.type = VARIANT_TYPE_STRING;
          TRACE_MSG("value n = " << out.string_value);
          out.string_value = pt.get<string>(inikey);
          break;

        case VARIANT_TYPE_STRING:
          out.string_value = pt.get<string>(inikey);
          TRACE_MSG("value s = " << out.string_value);
          break;
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      TRACE_MSG("e: " << e.what());
      ret = false;
    }

  TRACE_RETURN(ret);
  return ret;
}

bool
IniConfigurator::set_value(const std::string &key, Variant &value)
{
  TRACE_ENTER_MSG("IniConfigurator::set_value", key);
  bool ret = true;

  boost::property_tree::ptree::path_type inikey = path(key);

  try
    {
      switch (value.type)
        {
        case VARIANT_TYPE_INT:
          TRACE_MSG("value i = " << value.int_value);
          pt.put(inikey, value.int_value);
          break;

        case VARIANT_TYPE_BOOL:
          TRACE_MSG("value b = " << value.bool_value);
          pt.put(inikey, value.bool_value);
          break;

        case VARIANT_TYPE_DOUBLE:
          TRACE_MSG("value d = " << value.double_value);
          pt.put(inikey, value.double_value);
          break;

        case VARIANT_TYPE_NONE:
        case VARIANT_TYPE_STRING:
          TRACE_MSG("value s = " << value.string_value);
          pt.put(inikey, value.string_value);
          break;
        }
    }
  catch (boost::property_tree::ptree_error &e)
    {
      TRACE_MSG("e: " << e.what());
      ret = false;
    }

  return ret;
}

boost::property_tree::ptree::path_type
IniConfigurator::path(const string &key) const
{
  string new_key = key;
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
