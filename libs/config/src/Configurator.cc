// Copyright (C) 2002 - 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"
#include <cstdlib>
#include <sstream>

#include "Configurator.hh"

#include "utils/TimeSource.hh"

#include "IConfiguratorListener.hh"
#include "IConfigBackend.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;

// Constructs a new configurator.
Configurator::Configurator(IConfigBackend *backend)
  : backend(backend)
{
  if (dynamic_cast<IConfigBackendMonitoring *>(backend) != nullptr)
    {
      dynamic_cast<IConfigBackendMonitoring *>(backend)->set_listener(this);
    }
}

// Destructs the configurator.
Configurator::~Configurator()
{
  delete backend;
}

bool
Configurator::load(std::string filename)
{
  return backend->load(filename);
}

bool
Configurator::save(std::string filename)
{
  TRACE_ENTER_MSG("Configurator::save", filename);
  bool ret = backend->save(filename);
  TRACE_RETURN(ret);
  return ret;
}

bool
Configurator::save()
{
  TRACE_ENTER("Configurator::save");
  bool ret = backend->save();
  TRACE_RETURN(ret);
  return ret;
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
          Variant old_value;
          bool old_value_valid = backend->get_value(delayed.key, delayed.value.type, old_value);
          bool b = backend->set_value(delayed.key, delayed.value);

          if (b && dynamic_cast<IConfigBackendMonitoring *>(backend) == nullptr)
            {
              if (!old_value_valid || old_value != delayed.value)
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
  Setting setting;

  auto it = settings.find(key);
  if (it != settings.end())
    {
      Setting &setting = it->second;
      setting.delay = delay;
    }
  else
  {
    setting.key = key;
    setting.delay = delay;
    settings[key] = setting;
  }
}

bool
Configurator::has_user_value(const std::string &key)
{
  std::string newkey = key;
  strip_trailing_slash(newkey);
  strip_leading_slash(newkey);

  return backend->has_user_value(newkey);
}

bool
Configurator::remove_key(const std::string &key) const
{
  std::string newkey = key;
  strip_trailing_slash(newkey);
  strip_leading_slash(newkey);

  return backend->remove_key(newkey);
}

bool
Configurator::rename_key(const std::string &key, const std::string &new_key)
{
  bool ok = false;
  Variant value;

  bool exists = has_user_value(new_key);
  if (!exists)
    {
      ok = get_value(key, VARIANT_TYPE_NONE, value);
    }

  if (ok)
    {
      ok = set_value(new_key, value, CONFIG_FLAG_IMMEDIATE);
    }

  if (ok)
    {
      // Ignore error...
      remove_key(key);
    }

  return ok;
}

bool
Configurator::set_value(const std::string &key, Variant &value, ConfigFlags flags)
{
  bool ret = true;
  bool skip = false;
  Setting setting;
  string newkey;

  TRACE_ENTER_MSG("Configurator::set_value", key);

  if ((flags & CONFIG_FLAG_INITIAL) != 0)
    {
      skip = get_value(key, value.type, value);
    }

  if (!skip)
    {
      newkey = key;
      strip_trailing_slash(newkey);
      strip_leading_slash(newkey);
    }

  if (!skip && flags == CONFIG_FLAG_NONE)
    {
      bool b = find_setting(newkey, setting);
      if (b)
      {
          if (setting.delay)
            {
              DelayedConfig &d = delayed_config[newkey];
              d.key = newkey;
              d.value = value;
              d.until = TimeSource::get_monotonic_time_sec() + setting.delay;

              skip = true;
            }
        }
    }

  if (!skip)
    {
      Variant old_value;
      bool old_value_valid = backend->get_value(newkey, value.type, old_value);

      ret = backend->set_value(newkey, value);

      if (ret && dynamic_cast<IConfigBackendMonitoring *>(backend) == nullptr)
        {
          if (!old_value_valid || old_value != value)
            {
              fire_configurator_event(newkey);

              if (auto_save_time == 0)
                {
                  auto_save_time = TimeSource::get_monotonic_time_sec() + 30;
                }
            }
        }
    }

  TRACE_EXIT();
  return ret || skip;
}

bool
Configurator::get_value(const std::string &key, VariantType type, Variant &out) const
{
  bool ret = false;

  TRACE_ENTER_MSG("Configurator::get_value", key);

  string newkey = key;
  strip_trailing_slash(newkey);
  strip_leading_slash(newkey);

  auto it = delayed_config.find(newkey);
  if (it != delayed_config.end())
    {
      const DelayedConfig &delayed = it->second;
      out = delayed.value;
      ret = true;
    }

  if (!ret)
    {
      ret = backend->get_value(newkey, type, out);
    }

  if (ret && type != VARIANT_TYPE_NONE && out.type != type)
    {
      ret = false;
      out.type = VARIANT_TYPE_NONE;
    }

  TRACE_RETURN(ret);
  return ret;
}

bool
Configurator::get_value(const std::string &key, std::string &out) const
{
  Variant value;
  bool b = get_value(key, VARIANT_TYPE_STRING, value);

  if (b)
    {
      out = value.string_value;
    }

  return b;
}

bool
Configurator::get_value(const std::string &key, bool &out) const
{
  Variant value;
  bool b = get_value(key, VARIANT_TYPE_BOOL, value);

  if (b)
    {
      out = value.bool_value;
    }

  return b;
}

bool
Configurator::get_value(const std::string &key, int &out) const
{
  Variant value;
  bool b = get_value(key, VARIANT_TYPE_INT, value);

  if (b)
    {
      out = value.int_value;
    }

  return b;
}

bool
Configurator::get_value(const std::string &key, double &out) const
{
  Variant value;
  bool b = get_value(key, VARIANT_TYPE_DOUBLE, value);

  if (b)
    {
      out = value.double_value;
    }

  return b;
}

bool
Configurator::set_value(const std::string &key, const std::string &v, ConfigFlags flags)
{
  Variant value;
  bool ret = false;

  value.type = VARIANT_TYPE_STRING;
  value.string_value = v;
  ret = set_value(key, value, flags);

  return ret;
}

bool
Configurator::set_value(const std::string &key, const char *v, ConfigFlags flags)
{
  Variant value;
  bool ret = false;

  value.type = VARIANT_TYPE_STRING;
  value.string_value = v;
  ret = set_value(key, value, flags);

  return ret;
}

bool
Configurator::set_value(const std::string &key, int v, ConfigFlags flags)
{
  Variant value;
  bool ret = false;

  value.type = VARIANT_TYPE_INT;
  value.int_value = v;
  ret = set_value(key, value, flags);

  return ret;
}

bool
Configurator::set_value(const std::string &key, bool v, ConfigFlags flags)
{
  Variant value;
  bool ret = false;

  value.type = VARIANT_TYPE_BOOL;
  value.bool_value = v;
  ret = set_value(key, value, flags);

  return ret;
}

bool
Configurator::set_value(const std::string &key, double v, ConfigFlags flags)
{
  Variant value;
  bool ret = false;

  value.type = VARIANT_TYPE_DOUBLE;
  value.double_value = v;
  ret = set_value(key, value, flags);

  return ret;
}

void
Configurator::get_value_with_default(const string &key, int &out, const int def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const string &key, bool &out, const bool def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const string &key, string &out, const string def) const
{
  bool b = get_value(key, out);
  if (!b)
    {
      out = def;
    }
}

void
Configurator::get_value_with_default(const string &key, double &out, const double def) const
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
  string key = key_prefix;

  strip_leading_slash(key);
  strip_trailing_slash(key);

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
      listeners.push_back(make_pair(key, listener));
    }

  return ret;
}

bool
Configurator::remove_listener(IConfiguratorListener *listener)
{
  TRACE_ENTER("Configurator::remove_listener");
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
  TRACE_EXIT();
  return ret;
}

bool
Configurator::remove_listener(const std::string &key_prefix, IConfiguratorListener *listener)
{
  TRACE_ENTER("Configurator::remove_listener");
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
  TRACE_EXIT();
  return ret;
}

bool
Configurator::find_listener(IConfiguratorListener *listener, std::string &key) const
{
  bool ret = false;

  auto i = listeners.begin();
  while (i != listeners.end())
    {
      if (listener == i->second)
        {
          key = i->first;
          ret = true;
          break;
        }
      i++;
    }

  return ret;
}

//! Fire a configuration changed event.
void
Configurator::fire_configurator_event(const string &key)
{
  TRACE_ENTER_MSG("Configurator::fire_configurator_event", key);

  string k = key;
  strip_leading_slash(k);
  strip_trailing_slash(k);

  auto listeners_copy = listeners;

  auto i = listeners_copy.begin();
  while (i != listeners_copy.end())
    {
      string prefix = i->first;

      if (k.substr(0, prefix.length()) == prefix)
        {
          IConfiguratorListener *l = i->second;
          if (l != nullptr)
            {
              l->config_changed_notify(k);
            }
        }
      i++;
    }

  TRACE_EXIT();
}

//! Removes the leading '/'.
void
Configurator::strip_leading_slash(string &key) const
{
  size_t len = key.length();
  if (len > 1)
    {
      if (key[0] == '/')
        {
          key = key.substr(1, len - 1);
        }
    }
}

//! Removes the trailing '/'.
void
Configurator::strip_trailing_slash(string &key) const
{
  size_t len = key.length();
  if (len > 0)
    {
      if (key[len - 1] == '/')
        {
          key = key.substr(0, len - 1);
        }
    }
}

bool
Configurator::find_setting(const string &key, Setting &setting) const
{
  bool ret = false;

  auto it = settings.find(key);
  if (it != settings.end())
    {
      setting = it->second;
      ret = true;
    }

  return ret;
}

void
Configurator::config_changed_notify(const std::string &key)
{
  fire_configurator_event(key);
}
