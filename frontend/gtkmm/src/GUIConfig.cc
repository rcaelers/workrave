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
#include "config.h"
#endif

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include "GUIConfig.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "ICore.hh"
#include "IBreak.hh"

using namespace std;
using namespace workrave::config;

const string GUIConfig::CFG_KEY_BREAK_IGNORABLE    = "gui/breaks/%b/ignorable_break";
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


//!
void
GUIConfig::init()
{
  IConfigurator *config = CoreFactory::get_configurator();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      config->set_value(CFG_KEY_BREAK_IGNORABLE % ((BreakId)i),
                        true,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CFG_KEY_BREAK_EXERCISES % ((BreakId)i),
                        i == BREAK_ID_REST_BREAK ? 3 : 0,
                        CONFIG_FLAG_DEFAULT);

      config->set_value(CFG_KEY_BREAK_AUTO_NATURAL % ((BreakId)i),
                        false,
                        CONFIG_FLAG_DEFAULT);
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
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_BREAK_IGNORABLE % id,
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
  IBreak *b = CoreFactory::get_core()->get_break(id);

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


