// Copyright (C) 2002 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "Configurator.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "IConfiguratorListener.hh"
#include "IConfigBackend.hh"

#include "utils/TimeSource.hh"
#include "debug.hh"

using namespace workrave::utils;

Configurator::Configurator(IConfigBackend *backend)
  : backend(backend)
{
  if (dynamic_cast<IConfigBackendMonitoring *>(backend) != nullptr)
    {
      dynamic_cast<IConfigBackendMonitoring *>(backend)->set_listener(this);
    }
}

Configurator::~Configurator()
{
  delete backend;
}

bool
Configurator::load(std::string filename)
{
  return backend->load(filename);
}

void
Configurator::save()
{
  backend->save();
}

void
Configurator::heartbeat()
{
  int64_t now = TimeSource::get_monotonic_time_sec();

  auto it = delayed_config.begin();
  while (it != delayed_config.end())
    {
      DelayedConfig &delayed = it->second;
      auto next = it;
      next++;

      if (now >= delayed.until)
        {
          std::optional<ConfigValue> old_value = backend->get_value(delayed.key, ConfigValueToType(delayed.value));
          backend->set_value(delayed.key, delayed.value);

          if (dynamic_cast<IConfigBackendMonitoring *>(backend) == nullptr)
            {
              if (!old_value.has_value() || old_value != delayed.value)
                {
                  fire_configurator_event(delayed.key);
                  if (auto_save_time == 0)
                    {
                      auto_save_time = TimeSource::get_monotonic_time_sec() + 30;
                    }
                }
            }

          delayed_config.erase(it);
        }

      it = next;
    }

  if (auto_save_time != 0 && now >= auto_save_time)
    {
      save();
      auto_save_time = 0;
    }
}

void
Configurator::set_delay(const std::string &key, int delay)
{
  delays[key] = delay;
}

bool
Configurator::has_user_value(const std::string &key)
{
  return backend->has_user_value(trim_key(key));
}

void
Configurator::remove_key(const std::string &key) const
{
  backend->remove_key(trim_key(key));
}

void
Configurator::rename_key(const std::string &key, const std::string &new_key)
{
  std::optional<ConfigValue> current_value;

  bool exists = has_user_value(new_key);
  if (!exists)
    {
      current_value = get_value(key, workrave::config::ConfigType::Unknown);
    }

  if (current_value.has_value())
    {
      set_value(new_key, current_value.value(), workrave::config::CONFIG_FLAG_IMMEDIATE);
    }

  remove_key(key);
}

bool
Configurator::set_value(const std::string &key, ConfigValue &value, workrave::config::ConfigFlags flags)
{
  bool skip = false;

  std::string ckey = trim_key(key);

  if ((flags & workrave::config::CONFIG_FLAG_INITIAL) != 0)
    {
      auto current_value = get_value(ckey, ConfigValueToType(value));
      skip = current_value.has_value();
    }

  if (!skip && flags == workrave::config::CONFIG_FLAG_NONE)
    {
      if (delays.find(ckey) != delays.end() && delays[ckey] > 0)
        {
          DelayedConfig &d = delayed_config[ckey];
          d.key = ckey;
          d.value = value;
          d.until = TimeSource::get_monotonic_time_sec() + delays[ckey];

          skip = true;
        }
    }

  if (!skip)
    {
      auto current_value = get_value(ckey, ConfigValueToType(value));
      bool valid = current_value.has_value();
      backend->set_value(ckey, value);

      if (dynamic_cast<IConfigBackendMonitoring *>(backend) == nullptr)
        {
          if (!valid || current_value != value)
            {
              fire_configurator_event(ckey);

              if (auto_save_time == 0)
                {
                  auto_save_time = TimeSource::get_monotonic_time_sec() + 30;
                }
            }
        }
    }

  return skip;
}

std::optional<ConfigValue>
Configurator::get_value(const std::string &key, ConfigType type) const
{
  std::optional<ConfigValue> ret;

  std::string ckey = trim_key(key);

  auto it = delayed_config.find(ckey);
  if (it != delayed_config.end())
    {
      const DelayedConfig &delayed = it->second;
      ret = delayed.value;
    }

  if (!ret.has_value())
    {
      ret = backend->get_value(ckey, type);
    }

  return ret;
}

bool
Configurator::get_value(const std::string &key, std::string &out) const
{
  auto v = get_value(key, ConfigType::String);

  try
    {
      if (v.has_value())
        {
          std::visit([&out](auto &&arg) { out = boost::lexical_cast<std::string>(arg); }, v.value());
          return true;
        }
    }
  catch (boost::bad_lexical_cast &)
    {
    }

  return false;
}

bool
Configurator::get_value(const std::string &key, bool &out) const
{
  auto v = get_value(key, ConfigType::Boolean);

  try
    {
      if (v.has_value())
        {
          std::visit([&out](auto &&arg) { out = boost::lexical_cast<bool>(arg); }, v.value());
          return true;
        }
    }
  catch (boost::bad_lexical_cast &)
    {
    }

  return false;
}

bool
Configurator::get_value(const std::string &key, int32_t &out) const
{
  auto v = get_value(key, ConfigType::Int32);

  try
    {
      if (v.has_value())
        {
          std::visit([&out](auto &&arg) { out = boost::lexical_cast<int32_t>(arg); }, v.value());
          return true;
        }
    }
  catch (boost::bad_lexical_cast &)
    {
    }

  return false;
}

bool
Configurator::get_value(const std::string &key, int64_t &out) const
{
  auto v = get_value(key, ConfigType::Int64);

  try
    {
      if (v.has_value())
        {
          std::visit([&out](auto &&arg) { out = boost::lexical_cast<int64_t>(arg); }, v.value());
          return true;
        }
    }
  catch (boost::bad_lexical_cast &)
    {
    }

  return false;
}

bool
Configurator::get_value(const std::string &key, double &out) const
{
  auto v = get_value(key, ConfigType::Double);

  try
    {
      if (v.has_value())
        {
          std::visit([&out](auto &&arg) { out = boost::lexical_cast<double>(arg); }, v.value());
          return true;
        }
    }
  catch (boost::bad_lexical_cast &)
    {
    }

  return false;
}

void
Configurator::set_value(const std::string &key, const std::string &v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{v};
  set_value(key, value, flags);
}

void
Configurator::set_value(const std::string &key, const char *v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{std::string{v}};
  set_value(key, value, flags);
}

void
Configurator::set_value(const std::string &key, int32_t v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{v};
  set_value(key, value, flags);
}

void
Configurator::set_value(const std::string &key, int64_t v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{v};
  set_value(key, value, flags);
}

void
Configurator::set_value(const std::string &key, bool v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{v};
  set_value(key, value, flags);
}

void
Configurator::set_value(const std::string &key, double v, workrave::config::ConfigFlags flags)
{
  ConfigValue value{v};
  set_value(key, value, flags);
}

void
Configurator::get_value_with_default(const std::string &key, int32_t &out, int32_t def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const std::string &key, int64_t &out, int64_t def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const std::string &key, bool &out, bool def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const std::string &key, std::string &out, const std::string &def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const std::string &key, double &out, double def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

bool
Configurator::add_listener(const std::string &key_prefix, IConfiguratorListener *listener)
{
  bool ret = true;
  std::string key = trim_key(key_prefix);

  if (dynamic_cast<IConfigBackendMonitoring *>(backend) != nullptr)
    {
      ret = dynamic_cast<IConfigBackendMonitoring *>(backend)->add_listener(key_prefix);
    }

  if (ret)
    {
      auto i = listeners.begin();
      while (ret && i != listeners.end())
        {
          if (key == i->first && listener == i->second)
            {
              // Already added. Skip
              ret = false;
            }

          i++;
        }
    }

  if (ret)
    {
      // not found -> add
      listeners.emplace_back(key, listener);
    }

  return ret;
}

bool
Configurator::remove_listener(IConfiguratorListener *listener)
{
  TRACE_ENTRY();
  bool ret = false;

  auto i = listeners.begin();
  while (i != listeners.end())
    {
      if (listener == i->second)
        {
          // Found. Remove
          i = listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }
  return ret;
}

bool
Configurator::remove_listener(const std::string &key_prefix, IConfiguratorListener *listener)
{
  TRACE_ENTRY();
  bool ret = false;

  if (dynamic_cast<IConfigBackendMonitoring *>(backend) != nullptr)
    {
      dynamic_cast<IConfigBackendMonitoring *>(backend)->remove_listener(key_prefix);
    }

  auto i = listeners.begin();
  while (i != listeners.end())
    {
      if (i->first == key_prefix && i->second == listener)
        {
          // Found. Remove
          i = listeners.erase(i);
          ret = true;
        }
      else
        {
          i++;
        }
    }
  return ret;
}

//! Fire a configuration changed event.
void
Configurator::fire_configurator_event(const std::string &key)
{
  TRACE_ENTRY_PAR(key);

  std::string ckey = trim_key(key);

  auto listeners_copy = listeners;

  auto i = listeners_copy.begin();
  while (i != listeners_copy.end())
    {
      std::string prefix = i->first;

      if (ckey.substr(0, prefix.length()) == prefix)
        {
          IConfiguratorListener *l = i->second;
          if (l != nullptr)
            {
              l->config_changed_notify(ckey);
            }
        }
      i++;
    }
}

std::string
Configurator::trim_key(const std::string &key)
{
  return boost::trim_copy_if(key, boost::is_any_of("/"));
}

void
Configurator::config_changed_notify(const std::string &key)
{
  fire_configurator_event(key);
}
