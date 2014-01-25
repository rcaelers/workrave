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
#include "config.h"
#endif

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include "GUIConfig.hh"

#include "config/IConfigurator.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IBreak.hh"

using namespace std;
using namespace workrave::config;

const string GUIConfig::CFG_KEY_BREAK_IGNORABLE    = "gui/breaks/%b/ignorable_break";
const string GUIConfig::CFG_KEY_BREAK_SKIPPABLE    = "gui/breaks/%b/skippable_break";
const string GUIConfig::CFG_KEY_BREAK_EXERCISES    = "gui/breaks/%b/exercises";
const string GUIConfig::CFG_KEY_BREAK_AUTO_NATURAL = "gui/breaks/%b/auto_natural";
const string GUIConfig::CFG_KEY_BLOCK_MODE         = "gui/breaks/block_mode";
const string GUIConfig::CFG_KEY_LOCALE             = "gui/locale";
const string GUIConfig::CFG_KEY_TRAYICON_ENABLED   = "gui/trayicon_enabled";
const string GUIConfig::CFG_KEY_CLOSEWARN_ENABLED  = "gui/closewarn_enabled";

const string GUIConfig::CFG_KEY_MAIN_WINDOW               = "gui/main_window";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_START_IN_TRAY = "gui/main_window/start_in_tray";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_X             = "gui/main_window/x";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_Y             = "gui/main_window/y";
const string GUIConfig::CFG_KEY_MAIN_WINDOW_HEAD          = "gui/main_window/head";

const string GUIConfig::CFG_KEY_TIMERBOX = "gui/";
const string GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const string GUIConfig::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const string GUIConfig::CFG_KEY_TIMERBOX_POSITION = "/position";
const string GUIConfig::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const string GUIConfig::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";

//!
void
GUIConfig::init()
{
  IConfigurator::Ptr config = CoreFactory::get_configurator();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      BreakId breakId = (BreakId)i;
      
      config->set_value(CFG_KEY_BREAK_IGNORABLE % breakId,
                        true,
                        CONFIG_FLAG_INITIAL);

      config->set_value(CFG_KEY_BREAK_EXERCISES % breakId,
                        i == BREAK_ID_REST_BREAK ? 3 : 0,
                        CONFIG_FLAG_INITIAL);

      config->set_value(CFG_KEY_BREAK_AUTO_NATURAL % breakId,
                        false,
                        CONFIG_FLAG_INITIAL);

      // for backward compatibility with settings of older versions, we set the default
      // default value of `skippable` to whatever `ignorable`. This works because the old
      // meaning of `ignorable` was "show postpone and skip"; the new meaning is
      // "show postpone".
      bool ignorable;
      config->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % breakId, 
                                     ignorable, 
                                     true);

      config->set_value(CFG_KEY_BREAK_SKIPPABLE % breakId, 
                        ignorable, 
                        CONFIG_FLAG_INITIAL);
    }

  config->set_value(CFG_KEY_BLOCK_MODE, BLOCK_MODE_INPUT, CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_TRAYICON_ENABLED, true, CONFIG_FLAG_INITIAL);
  config->set_value(CFG_KEY_CLOSEWARN_ENABLED, true, CONFIG_FLAG_INITIAL);
}


//!
bool
GUIConfig::get_ignorable(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % id,
                             rc,
                             true);
  return rc;
}


//!
bool
GUIConfig::get_skippable(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_BREAK_SKIPPABLE % id,
                             rc,
                             true);
  return rc;
}

//!
void
GUIConfig::set_ignorable(BreakId id, bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_BREAK_IGNORABLE % id, b);
}

//!
bool
GUIConfig::get_trayicon_enabled()
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_TRAYICON_ENABLED,
                             rc,
                             true);
  return rc;
}


//!
void
GUIConfig::set_trayicon_enabled(bool b)
{
  CoreFactory::get_configurator()->set_value(CFG_KEY_TRAYICON_ENABLED, b);
}

//!
int
GUIConfig::get_number_of_exercises(BreakId id)
{
  int num;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_BREAK_EXERCISES % id,
                             num,
                             0);
  return num;
}


//!
void
GUIConfig::set_number_of_exercises(BreakId id, int num)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_BREAK_EXERCISES % id, num);
}


GUIConfig::BlockMode
GUIConfig::get_block_mode()
{
  int mode;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_BLOCK_MODE, mode, BLOCK_MODE_INPUT);
  return (BlockMode) mode;
}

void
GUIConfig::set_block_mode(BlockMode mode)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_BLOCK_MODE, int(mode));
}

std::string
GUIConfig::get_locale()
{
  string ret = "";
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_LOCALE, ret, "");

  return ret;
}

void
GUIConfig::set_locale(std::string locale)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_LOCALE, locale);
}


string
GUIConfig::expand(const string &key, BreakId id)
{
  IBreak::Ptr b = CoreFactory::get_core()->get_break(id);

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

bool
GUIConfig::get_always_on_top()
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP,
                             rc,
                             false);
  return rc;
}


void
GUIConfig::set_always_on_top(bool b)
{
  CoreFactory::get_configurator()
    ->set_value(GUIConfig::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
}


bool
GUIConfig::get_start_in_tray()
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, rc, false);
  return rc;
}


void
GUIConfig::set_start_in_tray(bool b)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, b);
}


int
GUIConfig::get_timerbox_cycle_time(string name)
{
  int ret;
  if (! CoreFactory::get_configurator()
      ->get_value(GUIConfig::CFG_KEY_TIMERBOX + name + GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME, ret))
    {
      ret = 10;
    }
  return ret;
}


void
GUIConfig::set_timerbox_cycle_time(string name, int time)
{
  CoreFactory::get_configurator()
    ->set_value(GUIConfig::CFG_KEY_TIMERBOX + name + GUIConfig::CFG_KEY_TIMERBOX_CYCLE_TIME, time);
}


const string
GUIConfig::get_timerbox_timer_config_key(string name, BreakId timer, const string &key)
{
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr break_data = core->get_break(BreakId(timer));

  return string(CFG_KEY_TIMERBOX) + name + "/" + break_data->get_name() + key;
}


int
GUIConfig::get_timerbox_timer_imminent_time(string name, BreakId timer)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  int ret;
  if (! CoreFactory::get_configurator()
      ->get_value(key, ret))
    {
      ret = 30;
    }
  return ret;
}


void
GUIConfig::set_timerbox_timer_imminent_time(string name, BreakId timer, int time)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  CoreFactory::get_configurator()->set_value(key, time);
}


int
GUIConfig::get_timerbox_timer_slot(string name, BreakId timer)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  int ret;
  if (! CoreFactory::get_configurator()
      ->get_value(key, ret))
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
GUIConfig::set_timerbox_timer_slot(string name, BreakId timer, int slot)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  CoreFactory::get_configurator()->set_value(key, slot);
}


int
GUIConfig::get_timerbox_timer_flags(string name, BreakId timer)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  int ret;
  if (! CoreFactory::get_configurator()
      ->get_value(key, ret))
    {
      ret = 0;
    }
  return ret;
}


void
GUIConfig::set_timerbox_timer_flags(string name, BreakId timer, int flags)
{
  const string key = get_timerbox_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  CoreFactory::get_configurator()->set_value(key, flags);
}


bool
GUIConfig::is_timerbox_enabled(string name)
{
  bool ret = true;
  if (! CoreFactory::get_configurator()
      ->get_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, ret))
    {
      ret = true;
    }
  return ret;
}


void
GUIConfig::set_timerbox_enabled(string name, bool enabled)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, enabled);
}
