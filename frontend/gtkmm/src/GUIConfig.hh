// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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
  static const std::string CFG_KEY_BREAK_ENABLE_SHUTDOWN;
  static const std::string CFG_KEY_BLOCK_MODE;
  static const std::string CFG_KEY_LOCALE;
  static const std::string CFG_KEY_TRAYICON_ENABLED;
  static const std::string CFG_KEY_AUTOSTART;
  static const std::string CFG_KEY_CLOSEWARN_ENABLED;
  static const std::string CFG_KEY_ICONTHEME;
  static const std::string CFG_KEY_FORCE_X11;

  static const std::string CFG_KEY_MAIN_WINDOW;
  static const std::string CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP;
  static const std::string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const std::string CFG_KEY_MAIN_WINDOW_X;
  static const std::string CFG_KEY_MAIN_WINDOW_Y;
  static const std::string CFG_KEY_MAIN_WINDOW_HEAD;

  static const std::string CFG_KEY_APPLET_FALLBACK_ENABLED;
  static const std::string CFG_KEY_APPLET_ICON_ENABLED;

  static void init();

  enum BlockMode
  {
    BLOCK_MODE_NONE = 0,
    BLOCK_MODE_INPUT,
    BLOCK_MODE_ALL
  };
  static BlockMode get_block_mode();
  static void set_block_mode(BlockMode mode);

  static std::string get_locale();
  static void set_locale(std::string locale);

  static bool get_trayicon_enabled();
  static void set_trayicon_enabled(bool enabled);

  static bool get_force_x11_enabled();
  static void set_force_x11_enabled(bool enabled);

  static bool get_ignorable(BreakId id);
  static bool get_skippable(BreakId id);
  static void set_ignorable(BreakId id, bool b);
  static bool get_shutdown_enabled(BreakId id);
  static int get_number_of_exercises(BreakId id);
  static void set_number_of_exercises(BreakId id, int num);

  static bool get_always_on_top();
  static void set_always_on_top(bool b);

  static void set_start_in_tray(bool b);
  static bool get_start_in_tray();

  static std::string get_icon_theme();
  static void set_icon_theme(std::string theme);

  static bool is_applet_fallback_enabled();
  static void set_applet_fallback_enabled(bool enabled);

  static bool is_applet_icon_enabled();
  static void set_applet_icon_enabled(bool enabled);

private:
  static std::string expand(const std::string &str, BreakId id);
};

#endif
