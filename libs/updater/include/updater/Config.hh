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

#ifndef WORKRAVE_UPDATER_CONFIG_HH
#define WORKRAVE_UPDATER_CONFIG_HH

#include "config/IConfigurator.hh"
#include "config/Setting.hh"

#include "utils/Enum.hh"

#include "unfold/Unfold.hh"

namespace workrave::updater
{
  enum class Channel
  {
    Stable,
    Candidate,
    Beta,
  };

  class Config
  {
  public:
    static void init(workrave::config::IConfigurator::Ptr configurator);
    static void deinit();

    static workrave::config::Setting<bool> &enabled();
    static workrave::config::Setting<int> &priority();
    static workrave::config::Setting<std::string, Channel> &channel();
    static workrave::config::Setting<std::string, unfold::ProxyType> &proxy_type();
    static workrave::config::Setting<std::string> &proxy();

  private:
    static workrave::config::IConfigurator::Ptr configurator;

    using sv = std::string_view;
    static constexpr std::string_view CFG_KEY_AUTO_UPDATES_ENABLED = sv("plugins/auto_update/enabled");
    static constexpr std::string_view CFG_KEY_AUTO_UPDATE_CHANNEL = sv("plugins/auto_update/channel");
    static constexpr std::string_view CFG_KEY_AUTO_UPDATE_PRIORITY = sv("plugins/auto_update/priority");
    static constexpr std::string_view CFG_KEY_AUTO_UPDATE_PROXY_TYPE = sv("plugins/auto_update/proxy_type");
    static constexpr std::string_view CFG_KEY_AUTO_UPDATE_PROXY = sv("plugins/auto_update/proxy");
  };

} // namespace workrave::updater

template<>
struct workrave::utils::enum_traits<workrave::updater::Channel>
{
  static constexpr auto min = workrave::updater::Channel::Stable;
  static constexpr auto max = workrave::updater::Channel::Beta;
  static constexpr auto linear = true;
  static constexpr std::array<std::pair<std::string_view, workrave::updater::Channel>, 5> names{
    {{"stable", workrave::updater::Channel::Stable},
     {"candidate", workrave::updater::Channel::Candidate},
     {"beta", workrave::updater::Channel::Beta}}};
};

inline std::ostream &
operator<<(std::ostream &stream, workrave::updater::Channel channel)
{
  stream << workrave::utils::enum_to_string(channel);
  return stream;
}

template<>
struct workrave::utils::enum_traits<unfold::ProxyType>
{
  static constexpr std::array<std::pair<std::string_view, unfold::ProxyType>, 3> names{
    {{"none", unfold::ProxyType::None}, {"system", unfold::ProxyType::System}, {"custom", unfold::ProxyType::Custom}}};
};

#endif
