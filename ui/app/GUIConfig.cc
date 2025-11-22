// Copyright (C) 2007 - 2013 Rob Caelers & Raymond Penners
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

#include "ui/GUIConfig.hh"

#include "config/IConfigurator.hh"
#include "config/SettingCache.hh"
#include "core/ICore.hh"
#include "core/IBreak.hh"

using namespace workrave::config;

const std::string GUIConfig::CFG_KEY_BREAK_IGNORABLE = "gui/breaks/%b/ignorable_break";
const std::string GUIConfig::CFG_KEY_BREAK_SKIPPABLE = "gui/breaks/%b/skippable_break";
const std::string GUIConfig::CFG_KEY_BREAK_ENABLE_SHUTDOWN = "gui/breaks/%b/enable_shutdown";
const std::string GUIConfig::CFG_KEY_BREAK_EXERCISES = "gui/breaks/%b/exercises";
const std::string GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL = "gui/breaks/%b/auto_natural";
const std::string GUIConfig::CFG_KEY_BLOCK_MODE = "gui/breaks/block_mode";
const std::string GUIConfig::CFG_KEY_FOCUS_MODE = "gui/breaks/focus_mode";
const std::string GUIConfig::CFG_KEY_FOLLOW_FOCUS_ASSIST_ENABLED = "gui/breaks/follow_focus_assist_enabled";
const std::string GUIConfig::CFG_KEY_LOCALE = "gui/locale";
const std::string GUIConfig::CFG_KEY_TRAYICON_ENABLED = "gui/trayicon_enabled";
const std::string GUIConfig::CFG_KEY_CLOSEWARN_ENABLED = "gui/closewarn_enabled";
const std::string GUIConfig::CFG_KEY_AUTOSTART = "gui/autostart";
const std::string GUIConfig::CFG_KEY_ICONTHEME = "gui/icontheme";
#if defined(PLATFORM_OS_WINDOWS)
const std::string GUIConfig::CFG_KEY_THEME_NAME = "gui/theme_name";
const std::string GUIConfig::CFG_KEY_THEME_DARK = "gui/theme_dark";
const std::string GUIConfig::CFG_KEY_LIGHT_DARK_MODE = "gui/light_dark_mode";
#endif
#if defined(PLATFORM_OS_UNIX)
const std::string GUIConfig::CFG_KEY_FORCE_X11 = "gui/force_x11";
const std::string GUIConfig::CFG_KEY_USE_GNOME_SHELL_PRELUDES = "gui/use_gnome_shell_preludes";
#endif
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW = "gui/main_window";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_START_IN_TRAY = "gui/main_window/start_in_tray";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_X = "gui/main_window/x";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_Y = "gui/main_window/y";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_HEAD = "gui/main_window/head";

const std::string GUIConfig::CFG_KEY_APPLET_FALLBACK_ENABLED = "gui/applet/fallback_enabled";
const std::string GUIConfig::CFG_KEY_APPLET_ICON_ENABLED = "gui/applet/icon_enabled";

const std::string GUIConfig::CFG_KEY_TIMERBOX = "gui/";
const std::string GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const std::string GUIConfig::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const std::string GUIConfig::CFG_KEY_TIMERBOX_POSITION = "/position";
const std::string GUIConfig::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const std::string GUIConfig::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";

std::string
GUIConfig::get_break_name(workrave::BreakId id)
{
  std::array<const char *, 3> names{"micro_pause", "rest_break", "daily_limit"};
  return names[(int)id];
}

void
GUIConfig::init(std::shared_ptr<workrave::config::IConfigurator> config)
{
  GUIConfig::config = config;

  for (int i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
    {
      auto breakId = static_cast<workrave::BreakId>(i);

      config->set_value(break_ignorable(breakId).key(), true, CONFIG_FLAG_INITIAL);
      config->set_value(break_exercises(breakId).key(), i == workrave::BREAK_ID_REST_BREAK ? 3 : 0, CONFIG_FLAG_INITIAL);
      config->set_value(break_auto_natural(breakId).key(), false, CONFIG_FLAG_INITIAL);

      // for backward compatibility with settings of older versions, we set the default
      // default value of `skippable` to whatever `ignorable`. This works because the old
      // meaning of `ignorable` was "show postpone and skip"; the new meaning is
      // "show postpone".
      bool ignorable{};
      config->get_value_with_default(break_ignorable(breakId).key(), ignorable, true);
      config->set_value(break_skippable(breakId).key(), ignorable, CONFIG_FLAG_INITIAL);
      config->set_value(break_enable_shutdown(breakId).key(), true, workrave::config::CONFIG_FLAG_INITIAL);
    }

  config->set_value(CFG_KEY_BLOCK_MODE,
                    static_cast<std::underlying_type<BlockMode>::type>(BlockMode::Input),
                    workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_FOCUS_MODE, 0, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_TRAYICON_ENABLED, true, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_CLOSEWARN_ENABLED, true, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_AUTOSTART, true, CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_LOCALE, "", CONFIG_FLAG_INITIAL);

#if defined(PLATFORM_OS_WINDOWS)
  bool dark = false;
  config->get_value_with_default(CFG_KEY_THEME_DARK, dark, false);
  config->set_value(CFG_KEY_LIGHT_DARK_MODE, dark ? 1 : 0, CONFIG_FLAG_INITIAL);
#endif
}

std::string
GUIConfig::expand(const std::string &key, workrave::BreakId id)
{
  std::string str = key;
  std::string::size_type pos = 0;
  std::string name = GUIConfig::get_break_name(id);

  while ((pos = str.find("%b", pos)) != std::string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  return str;
}

auto
GUIConfig::break_auto_natural(workrave::BreakId break_id) -> Setting<bool> &
{
  return SettingCache::get<bool>(config, expand(CFG_KEY_BREAK_AUTO_NATURAL, break_id));
}

auto
GUIConfig::break_ignorable(workrave::BreakId break_id) -> Setting<bool> &
{
  return SettingCache::get<bool>(config, expand(CFG_KEY_BREAK_IGNORABLE, break_id), true);
}

auto
GUIConfig::break_skippable(workrave::BreakId break_id) -> Setting<bool> &
{
  return SettingCache::get<bool>(config, expand(CFG_KEY_BREAK_SKIPPABLE, break_id), true);
}

auto
GUIConfig::break_enable_shutdown(workrave::BreakId break_id) -> Setting<bool> &
{
  return SettingCache::get<bool>(config, expand(CFG_KEY_BREAK_ENABLE_SHUTDOWN, break_id), true);
}

auto
GUIConfig::break_exercises(workrave::BreakId break_id) -> Setting<int> &
{
  return SettingCache::get<int>(config, expand(CFG_KEY_BREAK_EXERCISES, break_id), 0);
}

auto
GUIConfig::block_mode() -> Setting<int, BlockMode> &
{
  return SettingCache::get<int, BlockMode>(config, CFG_KEY_BLOCK_MODE, BlockMode::Input);
}

auto
GUIConfig::follow_focus_assist_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_FOLLOW_FOCUS_ASSIST_ENABLED, false);
}

auto
GUIConfig::focus_mode() -> Setting<int, FocusMode> &
{
  return SettingCache::get<int, FocusMode>(config, CFG_KEY_FOCUS_MODE, FocusMode::Quiet);
}

auto
GUIConfig::locale() -> Setting<std::string> &
{
  return SettingCache::get<std::string>(config, CFG_KEY_LOCALE, std::string());
}

auto
GUIConfig::trayicon_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_TRAYICON_ENABLED, true);
}

auto
GUIConfig::closewarn_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_CLOSEWARN_ENABLED);
}

auto
GUIConfig::autostart_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_AUTOSTART);
}

auto
GUIConfig::icon_theme() -> Setting<std::string> &
{
  return SettingCache::get<std::string>(config, CFG_KEY_ICONTHEME, std::string());
}

#if defined(PLATFORM_OS_WINDOWS)
auto
GUIConfig::theme_name() -> workrave::config::Setting<std::string> &
{
  return SettingCache::get<std::string>(config, CFG_KEY_THEME_NAME, std::string());
}

auto
GUIConfig::light_dark_mode() -> workrave::config::Setting<int, LightDarkTheme> &
{
  return SettingCache::get<int, LightDarkTheme>(config, CFG_KEY_LIGHT_DARK_MODE, LightDarkTheme::Auto);
}
#endif

#if defined(PLATFORM_OS_UNIX)
auto
GUIConfig::force_x11() -> workrave::config::Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_FORCE_X11, true);
}

auto
GUIConfig::use_gnome_shell_preludes() -> workrave::config::Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_USE_GNOME_SHELL_PRELUDES, true);
}
#endif

auto
GUIConfig::key_main_window() -> workrave::config::SettingGroup &
{
  return SettingCache::group(config, CFG_KEY_MAIN_WINDOW);
}

auto
GUIConfig::main_window_always_on_top() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, false);
}

auto
GUIConfig::main_window_start_in_tray() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_MAIN_WINDOW_START_IN_TRAY, false);
}

auto
GUIConfig::main_window_x() -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_MAIN_WINDOW_X, 256);
}

auto
GUIConfig::main_window_y() -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_MAIN_WINDOW_Y, 256);
}

auto
GUIConfig::main_window_head() -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_MAIN_WINDOW_HEAD, 0);
}

auto
GUIConfig::key_timerbox(const std::string &box) -> workrave::config::SettingGroup &
{
  return SettingCache::group(config, CFG_KEY_TIMERBOX + box);
}

auto
GUIConfig::timerbox_cycle_time(const std::string &box) -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_TIMERBOX + box + CFG_KEY_TIMERBOX_CYCLE_TIME, 10);
}

auto
GUIConfig::timerbox_slot(const std::string &box, workrave::BreakId break_id) -> Setting<int> &
{
  return SettingCache::get<int>(config,
                                CFG_KEY_TIMERBOX + box + "/" + get_break_name(break_id) + CFG_KEY_TIMERBOX_POSITION,
                                (box == "applet" ? 0 : break_id));
}

auto
GUIConfig::timerbox_flags(const std::string &box, workrave::BreakId break_id) -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_TIMERBOX + box + "/" + get_break_name(break_id) + CFG_KEY_TIMERBOX_FLAGS, 0);
}

auto
GUIConfig::timerbox_imminent(const std::string &box, workrave::BreakId break_id) -> Setting<int> &
{
  return SettingCache::get<int>(config, CFG_KEY_TIMERBOX + box + "/" + get_break_name(break_id) + CFG_KEY_TIMERBOX_IMMINENT, 30);
}

auto
GUIConfig::timerbox_enabled(const std::string &box) -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_TIMERBOX + box + CFG_KEY_TIMERBOX_ENABLED, true);
}

auto
GUIConfig::applet_fallback_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_APPLET_FALLBACK_ENABLED, false);
}

auto
GUIConfig::applet_icon_enabled() -> Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_APPLET_ICON_ENABLED, true);
}
