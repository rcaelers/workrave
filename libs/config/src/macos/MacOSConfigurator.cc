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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MacOSConfigurator.hh"

#include <string>

#include <objc/objc.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSString.h>

bool
MacOSConfigurator::load(std::string filename)
{
  (void)filename;
  return true;
}

void
MacOSConfigurator::save()
{
}

void
MacOSConfigurator::remove_key(const std::string &key)
{
  logger->debug("remove {}", key);
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];
  [[NSUserDefaults standardUserDefaults] removeObjectForKey:keystring];
}

bool
MacOSConfigurator::has_user_value(const std::string &key)
{
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];
  return ([[NSUserDefaults standardUserDefaults] objectForKey:keystring] != nil);
}

std::optional<ConfigValue>
MacOSConfigurator::get_value(const std::string &key, ConfigType type) const
{
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];

  if ([[NSUserDefaults standardUserDefaults] objectForKey:keystring] != nil)
    {
      switch (type)
        {
        case ConfigType::Int32:
          {
            int32_t v = [[NSUserDefaults standardUserDefaults] integerForKey:keystring];
            logger->debug("read {} = {}", key, v);
            return v;
          }

        case ConfigType::Int64:
          {
            int64_t v = [[NSUserDefaults standardUserDefaults] integerForKey:keystring];
            logger->debug("read {} = {}", key, v);
            return v;
          }

        case ConfigType::Boolean:
          {
            bool v = [[NSUserDefaults standardUserDefaults] boolForKey:keystring] == YES;
            logger->debug("read {} = {}", key, v);
            return v;
          }

        case ConfigType::Double:
          {
            double v = [[NSUserDefaults standardUserDefaults] doubleForKey:keystring];
            logger->debug("read {} = {}", key, v);
            return v;
          }

        case ConfigType::Unknown:
          [[fallthrough]];

        case ConfigType::String:
          {
            NSString *val = [[NSUserDefaults standardUserDefaults] stringForKey:keystring];
            if (val != nil)
              {
                std::string v = [val cStringUsingEncoding:NSUTF8StringEncoding];
                logger->debug("read {} = {}", key, v);
                return v;
              }
          }
        }
    }
  logger->warn("unknown setting: {}", key);
  return {};
}

void
MacOSConfigurator::set_value(const std::string &key, const ConfigValue &value)
{
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];

  std::visit(
    [key, keystring, this](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      logger->debug("write {} = {}", key, arg);
      if constexpr (std::is_same_v<bool, T>)
        {
          [[NSUserDefaults standardUserDefaults] setBool:arg forKey:keystring];
        }
      else if constexpr (std::is_same_v<int64_t, T>)
        {
          [[NSUserDefaults standardUserDefaults] setInteger:arg forKey:keystring];
        }
      else if constexpr (std::is_same_v<int32_t, T>)
        {
          [[NSUserDefaults standardUserDefaults] setInteger:arg forKey:keystring];
        }
      else if constexpr (std::is_same_v<double, T>)
        {
          [[NSUserDefaults standardUserDefaults] setDouble:arg forKey:keystring];
        }
      else if constexpr (std::is_same_v<std::string, T>)
        {
          NSString *string_value = [NSString stringWithCString:arg.c_str() encoding:NSASCIIStringEncoding];
          [[NSUserDefaults standardUserDefaults] setObject:string_value forKey:keystring];
        }
    },
    value);
}
