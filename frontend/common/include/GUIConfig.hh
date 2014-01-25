// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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
  static const std::string CFG_KEY_BREAK_SKIPPABLE;
  static const std::string CFG_KEY_BREAK_EXERCISES;
  static const std::string CFG_KEY_BLOCK_MODE;
  static const std::string CFG_KEY_LOCALE;
  static const std::string CFG_KEY_TRAYICON_ENABLED;
  static const std::string CFG_KEY_CLOSEWARN_ENABLED;

  static const std::string CFG_KEY_MAIN_WINDOW;
  static const std::string CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP;
  static const std::string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const std::string CFG_KEY_MAIN_WINDOW_X;
  static const std::string CFG_KEY_MAIN_WINDOW_Y;
  static const std::string CFG_KEY_MAIN_WINDOW_HEAD;

  static const std::string CFG_KEY_TIMERBOX;
  static const std::string CFG_KEY_TIMERBOX_HORIZONTAL;
  static const std::string CFG_KEY_TIMERBOX_CYCLE_TIME;
  static const std::string CFG_KEY_TIMERBOX_POSITION;
  static const std::string CFG_KEY_TIMERBOX_FLAGS;
  static const std::string CFG_KEY_TIMERBOX_IMMINENT;
  static const std::string CFG_KEY_TIMERBOX_ENABLED;
  
  static void init();

  enum BlockMode { BLOCK_MODE_NONE = 0, BLOCK_MODE_INPUT, BLOCK_MODE_ALL };
  static BlockMode get_block_mode();
  static void set_block_mode(BlockMode mode);

  static std::string get_locale();
  static void set_locale(std::string locale);

  static bool get_trayicon_enabled();
  static void set_trayicon_enabled(bool enabled);

  static bool get_ignorable(BreakId id);
  static bool get_skippable(BreakId id);
  static void set_ignorable(BreakId id, bool b);
  static int get_number_of_exercises(BreakId id);
  static void set_number_of_exercises(BreakId id, int num);

  static bool get_always_on_top();
  static void set_always_on_top(bool b);

  static void set_start_in_tray(bool b);
  static bool get_start_in_tray();

  static const std::string get_timerbox_timer_config_key(std::string name, workrave::BreakId timer, const std::string &key);
  static int get_timerbox_cycle_time(std::string name);
  static void set_timerbox_cycle_time(std::string name, int time);
  static int get_timerbox_timer_imminent_time(std::string name, workrave::BreakId timer);
  static void set_timerbox_timer_imminent_time(std::string name, workrave::BreakId timer, int time);
  static int get_timerbox_timer_slot(std::string name, workrave::BreakId timer);
  static void set_timerbox_timer_slot(std::string name, workrave::BreakId timer, int slot);

  enum SlotType
    {
      BREAK_WHEN_IMMINENT = 1,
      BREAK_WHEN_FIRST = 2,
      BREAK_SKIP = 4,
      BREAK_EXCLUSIVE = 8,
      BREAK_DEFAULT = 16,
      BREAK_HIDE = 32
    };

  static int get_timerbox_timer_flags(std::string name, workrave::BreakId timer);
  static void set_timerbox_timer_flags(std::string name, workrave::BreakId timer, int flags);
  static bool is_timerbox_enabled(std::string name);
  static void set_timerbox_enabled(std::string name, bool enabled);
 
private:
  static std::string expand(const std::string &str, BreakId id);
};

#endif
