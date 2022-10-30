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

using namespace std;
using namespace workrave::config;

const string GUIConfig::CFG_KEY_BREAK_IGNORABLE = "gui/breaks/%b/ignorable_break";
const string GUIConfig::CFG_KEY_BREAK_SKIPPABLE = "gui/breaks/%b/skippable_break";
const string GUIConfig::CFG_KEY_BREAK_ENABLE_SHUTDOWN = "gui/breaks/%b/enable_shutdown";
const string GUIConfig::CFG_KEY_BREAK_EXERCISES = "gui/breaks/%b/exercises";
const string GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL = "gui/breaks/%b/auto_natural";
const string GUIConfig::CFG_KEY_BLOCK_MODE = "gui/breaks/block_mode";
const string GUIConfig::CFG_KEY_FOCUS_MODE = "gui/breaks/focus_mode";
const string GUIConfig::CFG_KEY_FOLLOW_FOCUS_ASSIST_ENABLED = "gui/breaks/follow_focus_assist_enabled";
const string GUIConfig::CFG_KEY_LOCALE = "gui/locale";
const string GUIConfig::CFG_KEY_TRAYICON_ENABLED = "gui/trayicon_enabled";
const string GUIConfig::CFG_KEY_CLOSEWARN_ENABLED = "gui/closewarn_enabled";
const string GUIConfig::CFG_KEY_AUTOSTART = "gui/autostart";
const string GUIConfig::CFG_KEY_ICONTHEME = "gui/icontheme";
const string GUIConfig::CFG_KEY_THEME_NAME = "gui/theme_name";
const string GUIConfig::CFG_KEY_THEME_DARK = "gui/theme_dark";
const string GUIConfig::CFG_KEY_FORCE_X11 = "gui/force_x11";

const string GUIConfig::CFG_KEY_MAIN_WINDOW = "gui/main_window";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_START_IN_TRAY = "gui/main_window/start_in_tray";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_X = "gui/main_window/x";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_Y = "gui/main_window/y";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_HEAD = "gui/main_window/head";

const string GUIConfig::CFG_KEY_APPLET_FALLBACK_ENABLED = "gui/applet/fallback_enabled";
const string GUIConfig::CFG_KEY_APPLET_ICON_ENABLED = "gui/applet/icon_enabled";

const string GUIConfig::CFG_KEY_TIMERBOX = "gui/";
const string GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const string GUIConfig::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const string GUIConfig::CFG_KEY_TIMERBOX_POSITION = "/position";
const string GUIConfig::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const string GUIConfig::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";

void
GUIConfig::init(std::shared_ptr<IApplicationContext> app)
{
  GUIConfig::app = app;
  GUIConfig::config = app->get_core()->get_configurator();

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
}

auto
GUIConfig::expand(const string &key, workrave::BreakId id) -> string
{
  auto b = app->get_core()->get_break(id);

  string str = key;
  string::size_type pos = 0;
  string name = b->get_name();

  while ((pos = str.find("%b", pos)) != string::npos)
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

auto
GUIConfig::theme_dark() -> workrave::config::Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_THEME_DARK, false);
}

auto
GUIConfig::theme_name() -> workrave::config::Setting<std::string> &
{
  return SettingCache::get<std::string>(config, CFG_KEY_THEME_NAME, std::string());
}

auto
GUIConfig::force_x11() -> workrave::config::Setting<bool> &
{
  return SettingCache::get<bool>(config, CFG_KEY_FORCE_X11, true);
}

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
  auto br = app->get_core()->get_break(break_id);
  return SettingCache::get<int>(config,
                                CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_POSITION,
                                (box == "applet" ? 0 : break_id));
}

auto
GUIConfig::timerbox_flags(const std::string &box, workrave::BreakId break_id) -> Setting<int> &
{
  auto br = app->get_core()->get_break(break_id);
  return SettingCache::get<int>(config, CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_FLAGS, 0);
}

auto
GUIConfig::timerbox_imminent(const std::string &box, workrave::BreakId break_id) -> Setting<int> &
{
  auto br = app->get_core()->get_break(break_id);
  return SettingCache::get<int>(config, CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_IMMINENT, 30);
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
