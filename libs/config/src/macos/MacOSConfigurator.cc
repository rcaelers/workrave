// Copyright (C) 2008, 2009, 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSString.h>

#include <string>
#include <cstring>

#include "MacOSConfigurator.hh"

using namespace std;

bool
MacOSConfigurator::load(string filename)
{
  (void)filename;
  return true;
}

bool
MacOSConfigurator::save(string filename)
{
  (void)filename;
  return true;
}

bool
MacOSConfigurator::save()
{
  return true;
}

bool
MacOSConfigurator::remove_key(const std::string &key)
{
  TRACE_ENTER_MSG("MacOSConfigurator::remove_key", key);

  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];

  [[NSUserDefaults standardUserDefaults] removeObjectForKey:keystring];

  TRACE_EXIT();
  return true;
}

bool
MacOSConfigurator::has_user_value(const std::string &key)
{
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];
  return ([[NSUserDefaults standardUserDefaults] objectForKey:keystring] != nil);
}

bool
MacOSConfigurator::get_value(const std::string &key, VariantType type, Variant &out) const
{
  bool ret = false;

  TRACE_ENTER_MSG("MacOSConfigurator::get_value", key);

  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];
  out.type = type;

  if ([[NSUserDefaults standardUserDefaults] objectForKey:keystring] != nil)
    {
      ret = true;

      switch (type)
        {
        case VARIANT_TYPE_INT:
          out.int_value = [[NSUserDefaults standardUserDefaults] integerForKey:keystring];
          break;

        case VARIANT_TYPE_BOOL:
          out.bool_value = [[NSUserDefaults standardUserDefaults] boolForKey:keystring];
          break;

        case VARIANT_TYPE_DOUBLE:
          out.double_value = [[NSUserDefaults standardUserDefaults] floatForKey:keystring];
          break;

        case VARIANT_TYPE_NONE:
          out.type = VARIANT_TYPE_STRING;
          // FALLTHROUGH
          [[clang::fallthrough]];

        case VARIANT_TYPE_STRING:
          {
            NSString *val = [[NSUserDefaults standardUserDefaults] stringForKey:keystring];
            if (val != nil)
              {
                out.string_value = [val cStringUsingEncoding:NSUTF8StringEncoding];
              }
            else
              {
                ret = false;
              }
          }
          break;
        }
    }

  TRACE_RETURN(ret);
  return ret;
}

bool
MacOSConfigurator::set_value(const std::string &key, Variant &value)
{
  bool ret = true;

  TRACE_ENTER_MSG("MacOSConfigurator::get_value", key);
  NSString *keystring = [NSString stringWithCString:key.c_str() encoding:NSASCIIStringEncoding];

  switch (value.type)
    {
    case VARIANT_TYPE_INT:
      [[NSUserDefaults standardUserDefaults] setInteger:value.int_value forKey:keystring];
      break;

    case VARIANT_TYPE_BOOL:
      [[NSUserDefaults standardUserDefaults] setBool:value.bool_value forKey:keystring];
      break;

    case VARIANT_TYPE_DOUBLE:
      [[NSUserDefaults standardUserDefaults] setFloat:value.double_value forKey:keystring];
      break;

    case VARIANT_TYPE_NONE:
      ret = false;
      break;

    case VARIANT_TYPE_STRING:
      {
        string val = value.string_value;
        NSString *string_value = [NSString stringWithCString:val.c_str() encoding:NSASCIIStringEncoding];
        [[NSUserDefaults standardUserDefaults] setObject:string_value forKey:keystring];
      }
      break;
    }

  TRACE_RETURN(ret);
  return ret;
}
