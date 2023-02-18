// GUIConfig.cc --- The WorkRave GUI Configuration
//
// Copyright (C) 2007, 2008, 2010, 2011, 2012 Rob Caelers & Raymond Penners
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

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include "GUIConfig.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "ICore.hh"
#include "IBreak.hh"

const std::string GUIConfig::CFG_KEY_BREAK_IGNORABLE = "gui/breaks/%b/ignorable_break";
const std::string GUIConfig::CFG_KEY_BREAK_SKIPPABLE = "gui/breaks/%b/skippable_break";
const std::string GUIConfig::CFG_KEY_BREAK_EXERCISES = "gui/breaks/%b/exercises";
const std::string GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL = "gui/breaks/%b/auto_natural";
const std::string GUIConfig::CFG_KEY_BREAK_ENABLE_SHUTDOWN = "gui/breaks/%b/enable_shutdown";
const std::string GUIConfig::CFG_KEY_BLOCK_MODE = "gui/breaks/block_mode";
const std::string GUIConfig::CFG_KEY_LOCALE = "gui/locale";
const std::string GUIConfig::CFG_KEY_TRAYICON_ENABLED = "gui/trayicon_enabled";
const std::string GUIConfig::CFG_KEY_CLOSEWARN_ENABLED = "gui/closewarn_enabled";
const std::string GUIConfig::CFG_KEY_AUTOSTART = "gui/autostart";
const std::string GUIConfig::CFG_KEY_ICONTHEME = "gui/icontheme";
const std::string GUIConfig::CFG_KEY_FORCE_X11 = "gui/force_x11";

const std::string GUIConfig::CFG_KEY_MAIN_WINDOW = "gui/main_window";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_START_IN_TRAY = "gui/main_window/start_in_tray";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_X = "gui/main_window/x";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_Y = "gui/main_window/y";
const std::string GUIConfig::CFG_KEY_MAIN_WINDOW_HEAD = "gui/main_window/head";

const std::string GUIConfig::CFG_KEY_APPLET_FALLBACK_ENABLED = "gui/applet/fallback_enabled";
const std::string GUIConfig::CFG_KEY_APPLET_ICON_ENABLED = "gui/applet/icon_enabled";

//!
void
GUIConfig::init()
{
  IConfigurator *config = CoreFactory::get_configurator();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakId breakId = (BreakId)i;

      config->set_value(CFG_KEY_BREAK_IGNORABLE % breakId, true, CONFIG_FLAG_DEFAULT);

      config->set_value(CFG_KEY_BREAK_EXERCISES % breakId, i == BREAK_ID_REST_BREAK ? 3 : 0, CONFIG_FLAG_DEFAULT);

      config->set_value(CFG_KEY_BREAK_AUTO_NATURAL % breakId, false, CONFIG_FLAG_DEFAULT);

      // for backward compatibility with settings of older versions, we set the default
      // default value of `skippable` to whatever `ignorable`. This works because the old
      // meaning of `ignorable` was "show postpone and skip"; the new meaning is
      // "show postpone".
      bool ignorable;
      config->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % breakId, ignorable, true);

      config->set_value(CFG_KEY_BREAK_SKIPPABLE % breakId, ignorable, CONFIG_FLAG_DEFAULT);

      config->set_value(CFG_KEY_BREAK_ENABLE_SHUTDOWN % breakId, true, CONFIG_FLAG_DEFAULT);
    }

  config->set_value(CFG_KEY_BLOCK_MODE, BLOCK_MODE_INPUT, CONFIG_FLAG_DEFAULT);
  config->set_value(CFG_KEY_TRAYICON_ENABLED, true, CONFIG_FLAG_DEFAULT);
  config->set_value(CFG_KEY_CLOSEWARN_ENABLED, true, CONFIG_FLAG_DEFAULT);
}

//!
bool
GUIConfig::get_ignorable(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % id, rc, true);
  return rc;
}

//!
bool
GUIConfig::get_skippable(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_BREAK_SKIPPABLE % id, rc, true);
  return rc;
}

//!
void
GUIConfig::set_ignorable(BreakId id, bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_BREAK_IGNORABLE % id, b);
}

bool
GUIConfig::get_shutdown_enabled(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_BREAK_ENABLE_SHUTDOWN % id, rc, true);
  return rc;
}

//!
bool
GUIConfig::get_trayicon_enabled()
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_TRAYICON_ENABLED, rc, true);
  return rc;
}

//!
void
GUIConfig::set_trayicon_enabled(bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_TRAYICON_ENABLED, b);
}

bool
GUIConfig::get_force_x11_enabled()
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_FORCE_X11, rc, true);
  return rc;
}

//!
void
GUIConfig::set_force_x11_enabled(bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_FORCE_X11, b);
}

//!
int
GUIConfig::get_number_of_exercises(BreakId id)
{
  int num;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_BREAK_EXERCISES % id, num, 0);
  return num;
}

//!
void
GUIConfig::set_number_of_exercises(BreakId id, int num)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_BREAK_EXERCISES % id, num);
}

GUIConfig::BlockMode
GUIConfig::get_block_mode()
{
  int mode;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_BLOCK_MODE, mode, BLOCK_MODE_INPUT);
  return (BlockMode)mode;
}

void
GUIConfig::set_block_mode(BlockMode mode)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_BLOCK_MODE, int(mode));
}

std::string
GUIConfig::get_locale()
{
  std::string ret = "";
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_LOCALE, ret, "");

  return ret;
}

void
GUIConfig::set_locale(std::string locale)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_LOCALE, locale);
}

std::string
GUIConfig::expand(const std::string &key, BreakId id)
{
  IBreak *b = CoreFactory::get_core()->get_break(id);

  std::string str = key;
  std::string::size_type pos = 0;
  std::string name = b->get_name();

  while ((pos = str.find("%b", pos)) != std::string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  return str;
}

bool
GUIConfig::get_always_on_top()
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, rc, false);
  return rc;
}

void
GUIConfig::set_always_on_top(bool b)
{
  CoreFactory::get_configurator()->set_value(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
}

bool
GUIConfig::get_start_in_tray()
{
  bool rc;
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, rc, false);
  return rc;
}

void
GUIConfig::set_start_in_tray(bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, b);
}

std::string
GUIConfig::get_icon_theme()
{
  std::string ret = "";
  CoreFactory::get_configurator()->get_value_with_default(CFG_KEY_ICONTHEME, ret, "");

  return ret;
}

void
GUIConfig::set_icon_theme(std::string theme)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_ICONTHEME, theme);
}

bool
GUIConfig::is_applet_fallback_enabled()
{
  bool ret = true;
  if (!CoreFactory::get_configurator()->get_value(CFG_KEY_APPLET_FALLBACK_ENABLED, ret))
    {
      ret = false;
    }
  return ret;
}

void
GUIConfig::set_applet_fallback_enabled(bool enabled)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_APPLET_FALLBACK_ENABLED, enabled);
}

bool
GUIConfig::is_applet_icon_enabled()
{
  bool ret = true;
  if (!CoreFactory::get_configurator()->get_value(CFG_KEY_APPLET_ICON_ENABLED, ret))
    {
      ret = true;
    }
  return ret;
}

void
GUIConfig::set_applet_icon_enabled(bool enabled)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_APPLET_ICON_ENABLED, enabled);
}
