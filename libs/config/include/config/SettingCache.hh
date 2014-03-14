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

#ifndef WORKRAVE_CONFIG_SETTINGCACHE_HH
#define WORKRAVE_CONFIG_SETTINGCACHE_HH

#include <map>
#include <utility>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>

#include "config/IConfigurator.hh"
#include "config/Setting.hh"

namespace workrave
{
  namespace config
  {
    class SettingCache : public boost::noncopyable
    {
    public:
      template<typename T, typename S = T>
      static workrave::config::Setting<T, S> &get(IConfigurator::Ptr config, const std::string &key, const S &def = S())
      {
        if (cache.find(key) == cache.end()) 
          {
            cache[key] = new workrave::config::Setting<T, S>(config, key, def);
          } 
        
        workrave::config::Setting<T, S> *ret = dynamic_cast<workrave::config::Setting<T,S> *>(cache[key]);
        assert(ret != NULL);
        return *ret;
      }

      static workrave::config::SettingGroup &group(IConfigurator::Ptr config, const std::string &key)
      {
        if (cache.find(key) == cache.end()) 
          {
            cache[key] = new workrave::config::SettingGroup(config, key);
          } 
        
        workrave::config::SettingGroup *ret = dynamic_cast<workrave::config::SettingGroup *>(cache[key]);
        assert(ret != NULL);
        return *ret;
      }

    private:
      static std::map<std::string, SettingBase *> cache;
    };
  }
}

#endif // WORKRAVE_CONFIG_SETTINGCACHE_HH
