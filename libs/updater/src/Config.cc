// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#include <string>

#include "updater/Config.hh"

#include "config/SettingCache.hh"

using namespace workrave::updater;

workrave::config::IConfigurator::Ptr workrave::updater::Config::configurator;

void
Config::init(workrave::config::IConfigurator::Ptr config)
{
  Config::configurator = config;
}

workrave::config::Setting<bool> &
Config::enabled()
{
  return workrave::config::SettingCache::get<bool>(configurator, CFG_KEY_AUTO_UPDATES_ENABLED, true);
}

workrave::config::Setting<std::string, Channel> &
Config::channel()
{
  return workrave::config::SettingCache::get<std::string, Channel>(configurator, CFG_KEY_AUTO_UPDATE_CHANNEL, {});
}

workrave::config::Setting<std::string, unfold::ProxyType> &
Config::proxy_type()
{
  return workrave::config::SettingCache::get<std::string, unfold::ProxyType>(configurator,
                                                                             CFG_KEY_AUTO_UPDATE_PROXY_TYPE,
                                                                             unfold::ProxyType::System);
}

workrave::config::Setting<std::string> &
Config::proxy()
{
  return workrave::config::SettingCache::get<std::string>(configurator, CFG_KEY_AUTO_UPDATE_PROXY, {});
}
