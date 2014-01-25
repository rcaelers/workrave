// Copyright (C) 2013 Rob Caelers <rob.caelers@gmail.com>
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

#ifndef SETTING_HH
#define SETTING_HH

#include "config/IConfigurator.hh"

namespace workrave
{
  namespace config
  {
    template<class T, class R = T>
    class Setting
    {
    public:
      explicit Setting(workrave::config::IConfigurator::Ptr config, const std::string &setting) : config(config), setting(setting) , has_default_value(false)
      {
      }

      Setting(workrave::config::IConfigurator::Ptr config, const std::string &setting, R default_value) : config(config), setting(setting) , has_default_value(true), default_value(default_value)
      {
      }

      virtual ~Setting()
      {
      }

      const std::string key()
      {
        return setting;
      }

      const R operator()()
      {
        return get();
      }

      const R operator()(const T def)
      {
        return get(def);
      }

      const R get()
      {
        T ret = T();
        if (has_default_value)
          {
            config->get_value_with_default(setting, ret, static_cast<T>(default_value));
          }
        else
          {
            config->get_value(setting, ret);
          }
        return static_cast<R>(ret);
      }

      const R get(const R def)
      {
        const T ret = T();
        config->get_value_with_default(setting, ret, static_cast<T>(def));
        return static_cast<R>(ret);
      }

      void set(const R &val)
      {
        config->set_value(setting, static_cast<T>(val));
      }

    private:
      workrave::config::IConfigurator::Ptr config;
      std::string setting;
      bool has_default_value;
      R default_value;
    };
  }
}

#endif
