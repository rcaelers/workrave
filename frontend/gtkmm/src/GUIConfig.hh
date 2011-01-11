// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011 Rob Caelers & Raymond Penners
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

#ifndef GUICONFIG_HH
#define GUICONFIG_HH

#include "ICore.hh"

using namespace workrave;

class GUIConfig
{
public:
  static const std::string CFG_KEY_BREAK_AUTO_NATURAL;
  static const std::string CFG_KEY_BREAK_IGNORABLE;
  static const std::string CFG_KEY_BREAK_EXERCISES;
  static const std::string CFG_KEY_BLOCK_MODE;
  static const std::string CFG_KEY_LOCALE;
  static const std::string CFG_KEY_TRAYICON_ENABLED;

  static void init();

  enum BlockMode { BLOCK_MODE_NONE = 0, BLOCK_MODE_INPUT, BLOCK_MODE_ALL };
  static BlockMode get_block_mode();
  static void set_block_mode(BlockMode mode);

  static std::string get_locale();
  static void set_locale(std::string locale);

  static bool get_trayicon_enabled();
  static void set_trayicon_enabled(bool enabled);
  
  static bool get_ignorable(BreakId id);
  static void set_ignorable(BreakId id, bool b);
  static int get_number_of_exercises(BreakId id);
  static void set_number_of_exercises(BreakId id, int num);

private:

  static std::string expand(const std::string &str, BreakId id);
};

#endif
