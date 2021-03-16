// IConfigBackend.hh
//
// Copyright (C) 2001 - 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef CONFIGBACKENDADAPTER_HH
#define CONFIGBACKENDADAPTER_HH

#include <string>

#include "IConfigBackend.hh"

class ConfigBackendAdapter : public virtual IConfigBackend
{
public:
  virtual bool get_config_value(const std::string &key, std::string &out) const = 0;
  virtual bool get_config_value(const std::string &key, bool &out) const = 0;
  virtual bool get_config_value(const std::string &key, int &out) const = 0;
  virtual bool get_config_value(const std::string &key, long &out) const = 0;
  virtual bool get_config_value(const std::string &key, double &out) const = 0;

  virtual bool set_config_value(const std::string &key, std::string v) = 0;
  virtual bool set_config_value(const std::string &key, bool v) = 0;
  virtual bool set_config_value(const std::string &key, int v) = 0;
  virtual bool set_config_value(const std::string &key, long v) = 0;
  virtual bool set_config_value(const std::string &key, double v) = 0;

  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const
  {
    value.type = type;
    switch (type)
      {
      case VARIANT_TYPE_INT:
        return get_config_value(key, value.int_value);

      case VARIANT_TYPE_BOOL:
        return get_config_value(key, value.bool_value);

      case VARIANT_TYPE_DOUBLE:
        return get_config_value(key, value.double_value);

      case VARIANT_TYPE_NONE:
      case VARIANT_TYPE_STRING:
        return get_config_value(key, value.string_value);

      default:
        return false;
      }
  }

  virtual bool set_value(const std::string &key, Variant &value)
  {
    switch (value.type)
      {
      case VARIANT_TYPE_NONE:
        return false;

      case VARIANT_TYPE_INT:
        return set_config_value(key, value.int_value);

      case VARIANT_TYPE_BOOL:
        return set_config_value(key, value.bool_value);

      case VARIANT_TYPE_DOUBLE:
        return set_config_value(key, value.double_value);

      case VARIANT_TYPE_STRING:
        return set_config_value(key, value.string_value);

      default:
        return false;
      }
  }
};

#endif
