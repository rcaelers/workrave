// GUIConfig.cc
//
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

#include "GUIConfig.hh"

#include "config/IConfigurator.hh"
#include "config/SettingCache.hh"
#include "commonui/Backend.hh"
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
const string GUIConfig::CFG_KEY_LOCALE = "gui/locale";
const string GUIConfig::CFG_KEY_TRAYICON_ENABLED = "gui/trayicon_enabled";
const string GUIConfig::CFG_KEY_CLOSEWARN_ENABLED = "gui/closewarn_enabled";
const string GUIConfig::CFG_KEY_AUTOSTART = "gui/autostart";
const string GUIConfig::CFG_KEY_ICONTHEME = "gui/icontheme";

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
GUIConfig::init()
{
  IConfigurator::Ptr config = Backend::get_configurator();

  for (int i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
    {
      workrave::BreakId breakId = static_cast<workrave::BreakId>(i);

      config->set_value(break_ignorable(breakId).key(), true, CONFIG_FLAG_INITIAL);
      config->set_value(break_exercises(breakId).key(), i == workrave::BREAK_ID_REST_BREAK ? 3 : 0, CONFIG_FLAG_INITIAL);
      config->set_value(break_auto_natural(breakId).key(), false, CONFIG_FLAG_INITIAL);

      // for backward compatibility with settings of older versions, we set the default
      // default value of `skippable` to whatever `ignorable`. This works because the old
      // meaning of `ignorable` was "show postpone and skip"; the new meaning is
      // "show postpone".
      bool ignorable;
      config->get_value_with_default(break_ignorable(breakId).key(), ignorable, true);
      config->set_value(break_skippable(breakId).key(), ignorable, CONFIG_FLAG_INITIAL);

      config->set_value(CFG_KEY_BREAK_ENABLE_SHUTDOWN % breakId, true, workrave::config::CONFIG_FLAG_INITIAL);
    }

  config->set_value(CFG_KEY_BLOCK_MODE, BLOCK_MODE_INPUT, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_TRAYICON_ENABLED, true, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_CLOSEWARN_ENABLED, true, workrave::config::CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_AUTOSTART, true, CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_LOCALE, "", CONFIG_FLAG_INITIAL);
}

string
GUIConfig::expand(const string &key, workrave::BreakId id)
{
  auto b = Backend::get_core()->get_break(id);

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

Setting<bool> &
GUIConfig::break_auto_natural(workrave::BreakId break_id)
{
  return SettingCache::get<bool>(Backend::get_configurator(), expand(CFG_KEY_BREAK_AUTO_NATURAL, break_id));
}

Setting<bool> &
GUIConfig::break_ignorable(workrave::BreakId break_id)
{
  return SettingCache::get<bool>(Backend::get_configurator(), expand(CFG_KEY_BREAK_IGNORABLE, break_id), true);
}

Setting<bool> &
GUIConfig::break_skippable(workrave::BreakId break_id)
{
  return SettingCache::get<bool>(Backend::get_configurator(), expand(CFG_KEY_BREAK_SKIPPABLE, break_id), true);
}

Setting<bool> &
GUIConfig::break_enable_shutdown(workrave::BreakId break_id)
{
  return SettingCache::get<bool>(Backend::get_configurator(), expand(CFG_KEY_BREAK_ENABLE_SHUTDOWN, break_id), true);
}

Setting<int> &
GUIConfig::break_exercises(workrave::BreakId break_id)
{
  return SettingCache::get<int>(Backend::get_configurator(), expand(CFG_KEY_BREAK_EXERCISES, break_id), 0);
}

Setting<int, GUIConfig::BlockMode> &
GUIConfig::block_mode()
{
  return SettingCache::get<int, BlockMode>(Backend::get_configurator(), CFG_KEY_BLOCK_MODE, BLOCK_MODE_INPUT);
}

Setting<std::string> &
GUIConfig::locale()
{
  return SettingCache::get<std::string>(Backend::get_configurator(), CFG_KEY_LOCALE, std::string());
}

Setting<bool> &
GUIConfig::trayicon_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_TRAYICON_ENABLED, true);
}

Setting<bool> &
GUIConfig::closewarn_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_CLOSEWARN_ENABLED);
}

Setting<bool> &
GUIConfig::autostart_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_AUTOSTART);
}

Setting<std::string> &
GUIConfig::icon_theme()
{
  return SettingCache::get<std::string>(Backend::get_configurator(), CFG_KEY_ICONTHEME, std::string());
}

workrave::config::SettingGroup &
GUIConfig::key_main_window()
{
  return SettingCache::group(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW);
}

Setting<bool> &
GUIConfig::main_window_always_on_top()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, false);
}

Setting<bool> &
GUIConfig::main_window_start_in_tray()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW_START_IN_TRAY, false);
}

Setting<int> &
GUIConfig::main_window_x()
{
  return SettingCache::get<int>(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW_X, 256);
}

Setting<int> &
GUIConfig::main_window_y()
{
  return SettingCache::get<int>(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW_Y, 256);
}

Setting<int> &
GUIConfig::main_window_head()
{
  return SettingCache::get<int>(Backend::get_configurator(), CFG_KEY_MAIN_WINDOW_HEAD, 0);
}

workrave::config::SettingGroup &
GUIConfig::key_timerbox(const std::string &box)
{
  return SettingCache::group(Backend::get_configurator(), CFG_KEY_TIMERBOX + box);
}

Setting<int> &
GUIConfig::timerbox_cycle_time(const std::string &box)
{
  return SettingCache::get<int>(Backend::get_configurator(), CFG_KEY_TIMERBOX + box + CFG_KEY_TIMERBOX_CYCLE_TIME, 10);
}

Setting<int> &
GUIConfig::timerbox_slot(const std::string &box, workrave::BreakId break_id)
{
  auto br = Backend::get_core()->get_break(break_id);
  return SettingCache::get<int>(Backend::get_configurator(),
                                CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_POSITION,
                                (box == "applet" ? 0 : break_id));
}

Setting<int> &
GUIConfig::timerbox_flags(const std::string &box, workrave::BreakId break_id)
{
  auto br = Backend::get_core()->get_break(break_id);
  return SettingCache::get<int>(
    Backend::get_configurator(), CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_FLAGS, 0);
}

Setting<int> &
GUIConfig::timerbox_imminent(const std::string &box, workrave::BreakId break_id)
{
  auto br = Backend::get_core()->get_break(break_id);
  return SettingCache::get<int>(
    Backend::get_configurator(), CFG_KEY_TIMERBOX + box + "/" + br->get_name() + CFG_KEY_TIMERBOX_IMMINENT, 30);
}

Setting<bool> &
GUIConfig::timerbox_enabled(const std::string &box)
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_TIMERBOX + box + CFG_KEY_TIMERBOX_ENABLED, true);
}

Setting<bool> &
GUIConfig::applet_fallback_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_APPLET_FALLBACK_ENABLED, false);
}

Setting<bool> &
GUIConfig::applet_icon_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_APPLET_ICON_ENABLED, true);
}

#if !defined(HAVE_CORE_NEXT)
//!
bool
GUIConfig::get_ignorable(workrave::BreakId id)
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % id, rc, true);
  return rc;
}

//!
bool
GUIConfig::get_skippable(workrave::BreakId id)
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_BREAK_SKIPPABLE % id, rc, true);
  return rc;
}

//!
void
GUIConfig::set_ignorable(workrave::BreakId id, bool b)
{
  Backend::get_configurator()->set_value(CFG_KEY_BREAK_IGNORABLE % id, b);
}

bool
GUIConfig::get_shutdown_enabled(workrave::BreakId id)
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_BREAK_ENABLE_SHUTDOWN % id, rc, true);
  return rc;
}

//!
bool
GUIConfig::get_trayicon_enabled()
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_TRAYICON_ENABLED, rc, true);
  return rc;
}

//!
void
GUIConfig::set_trayicon_enabled(bool b)
{
  Backend::get_configurator()->set_value(CFG_KEY_TRAYICON_ENABLED, b);
}

//!
int
GUIConfig::get_number_of_exercises(workrave::BreakId id)
{
  int num;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_BREAK_EXERCISES % id, num, 0);
  return num;
}

//!
void
GUIConfig::set_number_of_exercises(workrave::BreakId id, int num)
{
  Backend::get_configurator()->set_value(CFG_KEY_BREAK_EXERCISES % id, num);
}

GUIConfig::BlockMode
GUIConfig::get_block_mode()
{
  int mode;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_BLOCK_MODE, mode, BLOCK_MODE_INPUT);
  return (BlockMode)mode;
}

void
GUIConfig::set_block_mode(BlockMode mode)
{
  Backend::get_configurator()->set_value(CFG_KEY_BLOCK_MODE, int(mode));
}

std::string
GUIConfig::get_locale()
{
  std::string ret = "";
  Backend::get_configurator()->get_value_with_default(CFG_KEY_LOCALE, ret, "");

  return ret;
}

void
GUIConfig::set_locale(std::string locale)
{
  Backend::get_configurator()->set_value(CFG_KEY_LOCALE, locale);
}


bool
GUIConfig::get_always_on_top()
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, rc, false);
  return rc;
}

void
GUIConfig::set_always_on_top(bool b)
{
  Backend::get_configurator()->set_value(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
}

bool
GUIConfig::get_start_in_tray()
{
  bool rc;
  Backend::get_configurator()->get_value_with_default(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, rc, false);
  return rc;
}

void
GUIConfig::set_start_in_tray(bool b)
{
  Backend::get_configurator()->set_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, b);
}

std::string
GUIConfig::get_icon_theme()
{
  std::string ret = "";
  Backend::get_configurator()->get_value_with_default(CFG_KEY_ICONTHEME, ret, "");

  return ret;
}

void
GUIConfig::set_icon_theme(std::string theme)
{
  Backend::get_configurator()->set_value(CFG_KEY_ICONTHEME, theme);
}

bool
GUIConfig::is_applet_fallback_enabled()
{
  bool ret = true;
  if (!Backend::get_configurator()->get_value(CFG_KEY_APPLET_FALLBACK_ENABLED, ret))
    {
      ret = false;
    }
  return ret;
}

void
GUIConfig::set_applet_fallback_enabled(bool enabled)
{
  Backend::get_configurator()->set_value(CFG_KEY_APPLET_FALLBACK_ENABLED, enabled);
}

bool
GUIConfig::is_applet_icon_enabled()
{
  bool ret = true;
  if (!Backend::get_configurator()->get_value(CFG_KEY_APPLET_ICON_ENABLED, ret))
    {
      ret = true;
    }
  return ret;
}

void
GUIConfig::set_applet_icon_enabled(bool enabled)
{
  Backend::get_configurator()->set_value(CFG_KEY_APPLET_ICON_ENABLED, enabled);
}

int
GUIConfig::get_timerbox_cycle_time(string name)
{
  int ret;
  if (!Backend::get_configurator()->get_value(
        GUIConfig::CFG_KEY_TIMERBOX + name + GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME, ret))
    {
      ret = 10;
    }
  return ret;
}

void
GUIConfig::set_timerbox_cycle_time(string name, int time)
{
  Backend::get_configurator()->set_value(
    GUIConfig::CFG_KEY_TIMERBOX + name + GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME, time);
}

const string
GUIConfig::get_timerbox_config_key(string name, workrave::BreakId timer, const string &key)
{
  auto core =  Backend::get_core();
  workrave::IBreak *break_data = core->get_break(workrave::BreakId(timer));

  return string(CFG_KEY_TIMERBOX) + name + "/" + break_data->get_name() + key;
}

int
GUIConfig::get_timerbox_imminent_time(string name, workrave::BreakId timer)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  int ret;
  if (!Backend::get_configurator()->get_value(key, ret))
    {
      ret = 30;
    }
  return ret;
}

void
GUIConfig::set_timerbox_imminent_time(string name, workrave::BreakId timer, int time)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  Backend::get_configurator()->set_value(key, time);
}

int
GUIConfig::get_timerbox_slot(string name, workrave::BreakId timer)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  int ret;
  if (!Backend::get_configurator()->get_value(key, ret))
    {
      if (name == "applet")
        {
          // All in one slot is probably the best default since we cannot assume
          // any users panel is large enough to hold all timers.
          ret = 0;
        }
      else
        {
          ret = timer;
        }
    }
  return ret;
}

void
GUIConfig::set_timerbox_slot(string name, workrave::BreakId timer, int slot)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  Backend::get_configurator()->set_value(key, slot);
}

int
GUIConfig::get_timerbox_flags(string name, workrave::BreakId timer)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  int ret;
  if (!Backend::get_configurator()->get_value(key, ret))
    {
      ret = 0;
    }
  return ret;
}

void
GUIConfig::set_timerbox_flags(string name, workrave::BreakId timer, int flags)
{
  const string key = get_timerbox_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  Backend::get_configurator()->set_value(key, flags);
}

bool
GUIConfig::is_timerbox_enabled(string name)
{
  bool ret = true;
  if (!Backend::get_configurator()->get_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, ret))
    {
      ret = true;
    }
  return ret;
}

void
GUIConfig::set_timerbox_enabled(string name, bool enabled)
{
  Backend::get_configurator()->set_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, enabled);
}

#endif
