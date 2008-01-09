// GUIConfig.cc --- The WorkRave GUI Configuration
//
// Copyright (C) 2007 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id: GUI.cc 1288 2007-08-24 09:22:23Z rcaelers $";

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

const string GUIConfig::CFG_KEY_BREAK_IGNORABLE = "breaks/%b/ignorable_break";
const string GUIConfig::CFG_KEY_BREAK_EXERCISES = "breaks/%b/exercises";
const string GUIConfig::CFG_KEY_GUI_BLOCK_MODE =  "breaks/block_mode";

//!
void
GUIConfig::init()
{
  IConfigurator *config = CoreFactory::get_configurator();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      config->set_value(expand(CFG_KEY_BREAK_IGNORABLE, (BreakId)i),
                        true, CONFIG_FLAG_DEFAULT);
      config->set_value(expand(CFG_KEY_BREAK_EXERCISES, (BreakId)i),
                        i == BREAK_ID_REST_BREAK ? 3 : 0,
                        CONFIG_FLAG_DEFAULT);
    }

  config->set_value(CFG_KEY_GUI_BLOCK_MODE, BLOCK_MODE_INPUT, CONFIG_FLAG_DEFAULT);
}


//!
bool
GUIConfig::get_ignorable(BreakId id)
{
  bool rc;
  CoreFactory::get_configurator()
    ->get_value_with_default(expand(CFG_KEY_BREAK_IGNORABLE, id),
                             rc,
                             true);
  return rc;
}


//!
void
GUIConfig::set_ignorable(BreakId id, bool b)
{
  CoreFactory::get_configurator()
    ->set_value(expand(CFG_KEY_BREAK_IGNORABLE, id),
                b);
}


//!
int
GUIConfig::get_number_of_exercises(BreakId id)
{
  int num;
  CoreFactory::get_configurator()
    ->get_value_with_default(expand(CFG_KEY_BREAK_EXERCISES, id),
                             num,
                             0);
  return num;
}


//!
void
GUIConfig::set_number_of_exercises(BreakId id, int num)
{
  CoreFactory::get_configurator()
    ->set_value(expand(CFG_KEY_BREAK_EXERCISES, id),
                          num);
}


GUIConfig::BlockMode
GUIConfig::get_block_mode()
{
  int mode;
  CoreFactory::get_configurator()
    ->get_value_with_default(CFG_KEY_GUI_BLOCK_MODE, mode, BLOCK_MODE_INPUT);
  return (BlockMode) mode;
}

void
GUIConfig::set_block_mode(BlockMode mode)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_GUI_BLOCK_MODE, int(mode));
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
