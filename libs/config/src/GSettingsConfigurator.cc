// Copyright (C) 2011 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "GSettingsConfigurator.hh"

#include <array>
#include <boost/algorithm/string/replace.hpp>

#include "Configurator.hh"

#include "debug.hh"

static const std::array<std::string, 2> underscore_exceptions = {
  "general/usage-mode",
  "general/operation-mode",
};

GSettingsConfigurator::GSettingsConfigurator()
{
  add_children();
}

GSettingsConfigurator::~GSettingsConfigurator()
{
  for (const auto &[key, setting]: settings)
    {
      g_object_unref(setting);
    }
}

bool
GSettingsConfigurator::load(std::string filename)
{
  (void)filename;
  return true;
}

void
GSettingsConfigurator::save()
{
}

void
GSettingsConfigurator::remove_key(const std::string &key)
{
  std::string subkey;
  GSettings *child = get_settings(key, subkey);
  g_settings_reset(child, subkey.c_str());
}

bool
GSettingsConfigurator::has_user_value(const std::string &key)
{
  std::string subkey;
  GSettings *child = get_settings(key, subkey);
  GVariant *value = g_settings_get_user_value(child, subkey.c_str());
  if (value != nullptr)
    {
      g_variant_unref(value);
      return true;
    }
  return false;
}

std::optional<ConfigValue>
GSettingsConfigurator::get_value(const std::string &key, ConfigType type) const
{
  std::string subkey;
  GSettings *child = get_settings(key, subkey);
  if (child == nullptr)
    {
      // TODO:log
      return {};
    }

  GVariant *value = g_settings_get_value(child, subkey.c_str());
  if (value == nullptr)
    {
      // TODO:log
      return {};
    }

  if (type == ConfigType::Unknown)
    {
      const GVariantType *value_type = g_variant_get_type(value);

      if (g_variant_type_equal(G_VARIANT_TYPE_INT32, value_type))
        {
          type = ConfigType::Int32;
        }
      else if (g_variant_type_equal(G_VARIANT_TYPE_INT64, value_type))
        {
          type = ConfigType::Int64;
        }
      else if (g_variant_type_equal(G_VARIANT_TYPE_BOOLEAN, value_type))
        {
          type = ConfigType::Boolean;
        }
      else if (g_variant_type_equal(G_VARIANT_TYPE_DOUBLE, value_type))
        {
          type = ConfigType::Double;
        }
      else if (g_variant_type_equal(G_VARIANT_TYPE_STRING, value_type))
        {
          type = ConfigType::String;
        }
    }

  const GVariantType *value_type = g_variant_get_type(value);
  g_variant_unref(value);

  if (type == ConfigType::Int32 && g_variant_type_equal(G_VARIANT_TYPE_INT32, value_type))
    {
      int32_t v = g_settings_get_int(child, subkey.c_str());
      return v;
    }
  if (type == ConfigType::Int64 && g_variant_type_equal(G_VARIANT_TYPE_INT64, value_type))
    {
      int64_t v = g_settings_get_int64(child, subkey.c_str());
      return v;
    }
  if (type == ConfigType::Boolean && g_variant_type_equal(G_VARIANT_TYPE_BOOLEAN, value_type))
    {
      bool v = (g_settings_get_boolean(child, subkey.c_str()) == TRUE);
      return v;
    }
  if (type == ConfigType::Double && g_variant_type_equal(G_VARIANT_TYPE_DOUBLE, value_type))
    {
      double v = g_settings_get_double(child, subkey.c_str());
      return v;
    }
  if (type == ConfigType::String && g_variant_type_equal(G_VARIANT_TYPE_STRING, value_type))
    {
      auto *str = g_settings_get_string(child, subkey.c_str());
      std::string v = str;
      g_free(str);
      return v;
    }
  return {};
}

void
GSettingsConfigurator::set_value(const std::string &key, const ConfigValue &value)
{
  std::string subkey;
  GSettings *child = get_settings(key, subkey);

  if (child == nullptr)
    {
      // TODO: log
      return;
    }

  std::visit(
    [child, subkey](auto &&value) {
      using T = std::decay_t<decltype(value)>;

      bool rc = false;
      if constexpr (std::is_same_v<bool, T>)
        {
          rc = g_settings_set_boolean(child, subkey.c_str(), value);
        }
      else if constexpr (std::is_same_v<int64_t, T>)
        {
          rc = g_settings_set_int64(child, subkey.c_str(), value);
        }
      else if constexpr (std::is_same_v<int32_t, T>)
        {
          rc = g_settings_set_int(child, subkey.c_str(), value);
        }
      else if constexpr (std::is_same_v<double, T>)
        {
          rc = g_settings_set_double(child, subkey.c_str(), value);
        }
      else if constexpr (std::is_same_v<std::string, T>)
        {
          rc = g_settings_set_string(child, subkey.c_str(), value.c_str());
        }

      if (!rc)
        {
          throw std::runtime_error("cannot set key");
        }
    },
    value);
}

void
GSettingsConfigurator::set_listener(workrave::config::IConfiguratorListener *listener)
{
  this->listener = listener;
}

bool
GSettingsConfigurator::add_listener(const std::string &key)
{
  (void)key;
  return true;
}

bool
GSettingsConfigurator::remove_listener(const std::string &remove_key)
{
  (void)remove_key;
  return true;
}

void
GSettingsConfigurator::add_children()
{
  TRACE_ENTRY();
  std::size_t len = schema_base.length();

  gchar **schemas = nullptr;
  g_settings_schema_source_list_schemas(g_settings_schema_source_get_default(), TRUE, &schemas, nullptr);

  for (int i = 0; schemas[i] != nullptr; i++)
    {
      if (g_ascii_strncasecmp(schemas[i], schema_base.c_str(), len) == 0)
        {
          GSettings *gsettings = g_settings_new(schemas[i]);

          settings[schemas[i]] = gsettings;
          g_signal_connect(gsettings, "changed", G_CALLBACK(on_settings_changed), this);
        }
    }
}

void
GSettingsConfigurator::on_settings_changed(GSettings *gsettings, const gchar *key, void *user_data)
{
  TRACE_ENTRY_PAR(key);
  gchar *path;
  g_object_get(gsettings, "path", &path, NULL);

  std::string tmp = boost::algorithm::replace_all_copy(std::string(path) + key, "/org/workrave/", "");
  std::string changed = boost::algorithm::replace_all_copy(tmp, "-", "_");
  TRACE_VAR(changed);

  for (const auto &exception: underscore_exceptions)
    {
      std::string mangled = boost::algorithm::replace_all_copy(exception, "-", "_");
      if (mangled == changed)
        {
          changed = exception;
          TRACE_MSG(" exception: {}", changed);
          break;
        }
    }

  auto *self = (GSettingsConfigurator *)user_data;
  if (self->listener != nullptr)
    {
      self->listener->config_changed_notify(changed);
    }

  g_free(path);
}

void
GSettingsConfigurator::key_split(const std::string &key, std::string &path, std::string &subkey)
{
  const char *s = key.c_str();
  const char *slash = strrchr(s, '/');
  if (slash != nullptr)
    {
      path = key.substr(0, slash - s);
      subkey = slash + 1;
    }
  else
    {
      path = "";
      subkey = "";
    }
}

GSettings *
GSettingsConfigurator::get_settings(const std::string &key, std::string &subkey) const
{
  TRACE_ENTRY_PAR(key);

  std::string path;

  std::string tmp = boost::algorithm::replace_all_copy(key, "_", "-");
  key_split(tmp, path, subkey);
  std::string schema = boost::algorithm::replace_all_copy(path, "/", ".");

  TRACE_VAR(subkey, path, schema);

  auto i = settings.find(schema_base + "." + schema);
  if (i == settings.end())
    {
      TRACE_MSG("NULL");
      return nullptr;
    }

  return i->second;
}
