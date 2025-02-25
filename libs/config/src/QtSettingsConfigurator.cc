// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2013 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "QtSettingsConfigurator.hh"

bool
QtSettingsConfigurator::load(std::string filename)
{
  (void)filename;
  return true;
}

void
QtSettingsConfigurator::save()
{
  settings.sync();
}

QString
QtSettingsConfigurator::qt_key(const std::string &key)
{
  QString qkey(key.c_str());
  return qkey.prepend('/');
}

QVariant
QtSettingsConfigurator::qt_get_value(const std::string &key) const
{
  QVariant var;
  const QString qkey = qt_key(key);
  if (settings.contains(qkey))
    {
      var = settings.value(qkey);
    }
  return var;
}

bool
QtSettingsConfigurator::has_user_value(const std::string &key)
{
  QVariant var = qt_get_value(key);
  return var.isValid();
}

void
QtSettingsConfigurator::remove_key(const std::string &key)
{
  const QString qkey = qt_key(key);
  settings.remove(qkey);
}

std::optional<ConfigValue>
QtSettingsConfigurator::get_value(const std::string &key, ConfigType type) const
{
  QVariant var = qt_get_value(key);
  logger->debug("read {} = {}", key, var.toString().toStdString());
  if (var.isValid())
    {
      switch (type)
        {
        case ConfigType::Int32:
          return var.toInt();

        case ConfigType::Int64:
          return var.toLongLong();

        case ConfigType::Boolean:
          return var.toBool();

        case ConfigType::Double:
          return var.toDouble();

        case ConfigType::Unknown:
          [[fallthrough]];

        case ConfigType::String:
          return var.toString().toStdString();
        }
    }
  return {};
}

void
QtSettingsConfigurator::set_value(const std::string &key, const ConfigValue &value)
{
  std::visit(
    [key, this](auto &&value) {
      using T = std::decay_t<decltype(value)>;

      const QString qkey = qt_key(key);

      if constexpr (std::is_same_v<std::string, T>)
        {
          QVariant qval = value.c_str();
          settings.setValue(qkey, qval);
        }
      else if constexpr (!std::is_same_v<std::monostate, T>)
        {
          QVariant qval = value;
          settings.setValue(qkey, qval);
        }
    },
    value);
}
