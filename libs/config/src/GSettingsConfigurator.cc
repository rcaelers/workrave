// GSettingsConfigurator.cc --- Configuration Access
//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_GSETTINGS

#  include "debug.hh"

#  include "GSettingsConfigurator.hh"
#  include "Configurator.hh"
#  include "utils/StringUtil.hh"

using namespace workrave;
using namespace std;

static string underscore_exceptions[] = {
  "general/usage-mode",
  "general/operation-mode",
};

GSettingsConfigurator::GSettingsConfigurator()
{
  schema_base = "org.workrave";
  path_base = "/org/workrave/";

  add_children();
}

bool
GSettingsConfigurator::load(string filename)
{
  (void)filename;
  return true;
}

bool
GSettingsConfigurator::save(string filename)
{
  (void)filename;
  return true;
}

bool
GSettingsConfigurator::save()
{
  return true;
}

bool
GSettingsConfigurator::remove_key(const std::string &key)
{
  bool ret = true;
  (void)key;
  return ret;
}

bool
GSettingsConfigurator::get_value(const std::string &full_path, VariantType type, Variant &out) const
{
  bool ret = false;

  string key;
  GSettings *child = get_settings(full_path, key);
  if (child != nullptr)
    {
      GVariant *value = g_settings_get_value(child, key.c_str());
      if (value != nullptr)
        {
          if (type == VARIANT_TYPE_NONE)
            {
              const GVariantType *value_type = g_variant_get_type(value);

              if (g_variant_type_equal(G_VARIANT_TYPE_INT32, value_type))
                {
                  type = VARIANT_TYPE_INT;
                }
              else if (g_variant_type_equal(G_VARIANT_TYPE_BOOLEAN, value_type))
                {
                  type = VARIANT_TYPE_BOOL;
                }
              else if (g_variant_type_equal(G_VARIANT_TYPE_DOUBLE, value_type))
                {
                  type = VARIANT_TYPE_DOUBLE;
                }
              else if (g_variant_type_equal(G_VARIANT_TYPE_STRING, value_type))
                {
                  type = VARIANT_TYPE_STRING;
                }
            }

          ret = false;
          const GVariantType *value_type = g_variant_get_type(value);

          if (g_variant_type_equal(G_VARIANT_TYPE_INT32, value_type))
            {
              out.int_value = g_settings_get_int(child, key.c_str());
              ret = true;
            }
          else if (g_variant_type_equal(G_VARIANT_TYPE_BOOLEAN, value_type))
            {
              out.bool_value = g_settings_get_boolean(child, key.c_str());
              ret = true;
            }
          else if (g_variant_type_equal(G_VARIANT_TYPE_DOUBLE, value_type))
            {
              out.double_value = g_settings_get_double(child, key.c_str());
              ret = true;
            }
          else if (g_variant_type_equal(G_VARIANT_TYPE_STRING, value_type))
            {
              out.string_value = g_settings_get_string(child, key.c_str());
              ret = true;
            }

          // g_variant_unref(value);
        }

      if (ret)
        {
          out.type = type;
        }
    }

  return ret;
}

bool
GSettingsConfigurator::set_value(const std::string &full_path, Variant &value)
{
  bool ret = true;

  string key;
  GSettings *child = get_settings(full_path, key);

  if (child != nullptr)
    {
      switch (value.type)
        {
        case VARIANT_TYPE_NONE:
          ret = false;
          break;

        case VARIANT_TYPE_INT:
          ret = g_settings_set_int(child, key.c_str(), value.int_value);
          break;

        case VARIANT_TYPE_BOOL:
          ret = g_settings_set_boolean(child, key.c_str(), value.bool_value);
          break;

        case VARIANT_TYPE_DOUBLE:
          ret = g_settings_set_double(child, key.c_str(), value.double_value);
          break;

        case VARIANT_TYPE_STRING:
          ret = g_settings_set_string(child, key.c_str(), value.string_value.c_str());
          break;

        default:
          ret = false;
        }
    }
  return ret;
}

void
GSettingsConfigurator::set_listener(IConfiguratorListener *listener)
{
  this->listener = listener;
}

bool
GSettingsConfigurator::add_listener(const string &key)
{
  (void)key;
  return true;
}

bool
GSettingsConfigurator::remove_listener(const string &remove_key)
{
  (void)remove_key;
  return true;
}

void
GSettingsConfigurator::add_children()
{
  TRACE_ENTER("GSettingsConfigurator::add_children");
  int len = schema_base.length();

  GSettingsSchemaSource *global_schema_source = g_settings_schema_source_get_default();

  gchar **schemas = nullptr;
  g_settings_schema_source_list_schemas(global_schema_source, TRUE, &schemas, nullptr);

  for (int i = 0; schemas[i] != nullptr; i++)
    {
      if (g_ascii_strncasecmp(schemas[i], schema_base.c_str(), len) == 0)
        {
          GSettings *gsettings = g_settings_new(schemas[i]);

          settings[schemas[i]] = gsettings;
          g_signal_connect(gsettings, "changed", G_CALLBACK(on_settings_changed), this);
        }
    }

  TRACE_EXIT();
}

void
GSettingsConfigurator::on_settings_changed(GSettings *gsettings, const gchar *key, void *user_data)
{
  TRACE_ENTER_MSG("GSettingsConfigurator::on_settings_changed", key);
  gchar *path;
  g_object_get(gsettings, "path", &path, NULL);

  string tmp = StringUtil::search_replace(string(path) + key, "/org/workrave/", "");
  string changed = StringUtil::search_replace(tmp, "-", "_");
  TRACE_MSG(changed);

  for (unsigned int i = 0; i < sizeof(underscore_exceptions) / sizeof(string); i++)
    {
      string mangled = StringUtil::search_replace(underscore_exceptions[i], "-", "_");
      if (mangled == changed)
        {
          changed = underscore_exceptions[i];
          TRACE_MSG(" exception: " << changed);
          break;
        }
    }

  GSettingsConfigurator *self = (GSettingsConfigurator *)user_data;
  self->listener->config_changed_notify(changed);

  g_free(path);
  TRACE_EXIT();
}

void
GSettingsConfigurator::key_split(const string &key, string &parent, string &child) const
{
  const char *s = key.c_str();
  const char *slash = strrchr(s, '/');
  if (slash)
    {
      parent = key.substr(0, slash - s);
      child = slash + 1;
    }
  else
    {
      parent = "";
      child = "";
    }
}

GSettings *
GSettingsConfigurator::get_settings(const std::string &full_path, string &key) const
{
  TRACE_ENTER_MSG("GSettingsConfigurator::get_settings", full_path);

  string path;
  key_split(StringUtil::search_replace(full_path, "_", "-"), path, key);
  string schema = StringUtil::search_replace(path, "/", ".");

  TRACE_MSG(key << " " << path << " " << schema);

  SettingsCIter i = settings.find(schema_base + "." + schema);
  if (i == settings.end())
    {
      TRACE_RETURN("NULL");
      return nullptr;
    }

  TRACE_EXIT();
  return i->second;
}

#endif
