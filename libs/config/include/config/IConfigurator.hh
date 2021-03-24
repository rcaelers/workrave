// IConfigurator.hh -- Interface to the Workrave configuration
//
// Copyright (C) 2001 - 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef ICONFIGURATOR_HH
#define ICONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

namespace workrave
{

  // Forward declaratons
  class IConfiguratorListener;

  //! Hints on how to set a configuration value.
  enum ConfigFlags
  {
    //! No special hints.
    CONFIG_FLAG_NONE = 0,

    //! The default value is set.
    CONFIG_FLAG_DEFAULT = 1,

    //! The value must be set immediately, without delay.
    CONFIG_FLAG_IMMEDIATE = 2,
  };

  //! Interface to access the configuration.
  class IConfigurator
  {
  public:
    virtual ~IConfigurator() = default;

    virtual void heartbeat() = 0;

    virtual void set_delay(const std::string &key, int delay) = 0;

    virtual bool load(std::string filename) = 0;
    virtual bool save(std::string filename) = 0;
    virtual bool save() = 0;

    virtual bool remove_key(const std::string &key) const = 0;
    virtual bool rename_key(const std::string &key, const std::string &new_key) = 0;

    virtual bool get_value(const std::string &key, std::string &out) const = 0;
    virtual bool get_value(const std::string &key, bool &out) const = 0;
    virtual bool get_value(const std::string &key, int &out) const = 0;
    virtual bool get_value(const std::string &key, double &out) const = 0;

    virtual void get_value_with_default(const std::string &key, std::string &out, std::string s) const = 0;
    virtual void get_value_with_default(const std::string &key, bool &out, const bool def) const = 0;
    virtual void get_value_with_default(const std::string &key, int &out, const int def) const = 0;
    virtual void get_value_with_default(const std::string &key, double &out, const double def) const = 0;

    virtual bool set_value(const std::string &key, const std::string &v, ConfigFlags flags = CONFIG_FLAG_NONE) = 0;
    virtual bool set_value(const std::string &key, const char *v, ConfigFlags flags = CONFIG_FLAG_NONE) = 0;
    virtual bool set_value(const std::string &key, int v, ConfigFlags flags = CONFIG_FLAG_NONE) = 0;
    virtual bool set_value(const std::string &key, bool v, ConfigFlags flags = CONFIG_FLAG_NONE) = 0;
    virtual bool set_value(const std::string &key, double v, ConfigFlags flags = CONFIG_FLAG_NONE) = 0;

    virtual bool get_typed_value(const std::string &key, std::string &t) const = 0;
    virtual bool set_typed_value(const std::string &key, const std::string &t) = 0;

    virtual bool add_listener(const std::string &key_prefix, IConfiguratorListener *listener) = 0;
    virtual bool remove_listener(IConfiguratorListener *listener) = 0;
    virtual bool remove_listener(const std::string &key_prefix, IConfiguratorListener *listener) = 0;
    virtual bool find_listener(IConfiguratorListener *listener, std::string &key) const = 0;
  };
} // namespace workrave

#endif // ICONFIGURATOR_HH
