// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#ifndef WORKRAVE_UI_GUICONFIG_HH
#define WORKRAVE_UI_GUICONFIG_HH

#include <iostream>

#include "config/IConfigurator.hh"
#include "core/CoreTypes.hh"
#include "config/Setting.hh"

enum class BlockMode
{
  Off = 0,
  Input,
  All
};

enum class FocusMode
{
  Suspended,
  Quiet
};

enum class LightDarkTheme
{
  Light,
  Dark,
  Auto
};

class GUIConfig
{
public:
  enum SlotType
  {
    BREAK_WHEN_IMMINENT = 1,
    BREAK_WHEN_FIRST = 2,
    BREAK_SKIP = 4,
    BREAK_EXCLUSIVE = 8,
    BREAK_DEFAULT = 16,
    BREAK_HIDE = 32
  };

  static workrave::config::Setting<bool> &break_auto_natural(workrave::BreakId break_id);
  static workrave::config::Setting<bool> &break_ignorable(workrave::BreakId break_id);
  static workrave::config::Setting<bool> &break_skippable(workrave::BreakId break_id);
  static workrave::config::Setting<bool> &break_enable_shutdown(workrave::BreakId break_id);
  static workrave::config::Setting<int> &break_exercises(workrave::BreakId break_id);
  static workrave::config::Setting<int, BlockMode> &block_mode();
  static workrave::config::Setting<bool> &follow_focus_assist_enabled();
  static workrave::config::Setting<int, FocusMode> &focus_mode();
  static workrave::config::Setting<std::string> &locale();
  static workrave::config::Setting<bool> &trayicon_enabled();
  static workrave::config::Setting<bool> &closewarn_enabled();
  static workrave::config::Setting<bool> &autostart_enabled();
  static workrave::config::Setting<std::string> &icon_theme();
  static workrave::config::Setting<std::string> &theme_name();
  static workrave::config::Setting<int, LightDarkTheme> &light_dark_mode();
  static workrave::config::Setting<bool> &force_x11();

  static workrave::config::Setting<bool> &main_window_always_on_top();
  static workrave::config::Setting<bool> &main_window_start_in_tray();
  static workrave::config::Setting<int> &main_window_x();
  static workrave::config::Setting<int> &main_window_y();
  static workrave::config::Setting<int> &main_window_head();

  static workrave::config::Setting<bool> &applet_fallback_enabled();
  static workrave::config::Setting<bool> &applet_icon_enabled();

  static workrave::config::Setting<int> &timerbox_cycle_time(const std::string &box);
  static workrave::config::Setting<int> &timerbox_slot(const std::string &box, workrave::BreakId break_id);
  static workrave::config::Setting<int> &timerbox_flags(const std::string &box, workrave::BreakId break_id);
  static workrave::config::Setting<int> &timerbox_imminent(const std::string &box, workrave::BreakId break_id);
  static workrave::config::Setting<bool> &timerbox_enabled(const std::string &box);

  static workrave::config::SettingGroup &key_main_window();
  static workrave::config::SettingGroup &key_timerbox(const std::string &box);

  static void init(workrave::config::IConfigurator::Ptr config);

private:
  static const std::string CFG_KEY_BREAK_AUTO_NATURAL;
  static const std::string CFG_KEY_BREAK_IGNORABLE;
  static const std::string CFG_KEY_BREAK_SKIPPABLE;
  static const std::string CFG_KEY_BREAK_EXERCISES;
  static const std::string CFG_KEY_BREAK_ENABLE_SHUTDOWN;
  static const std::string CFG_KEY_BLOCK_MODE;
  static const std::string CFG_KEY_FOCUS_MODE;
  static const std::string CFG_KEY_FOLLOW_FOCUS_ASSIST_ENABLED;
  static const std::string CFG_KEY_LOCALE;
  static const std::string CFG_KEY_TRAYICON_ENABLED;
  static const std::string CFG_KEY_AUTOSTART;
  static const std::string CFG_KEY_CLOSEWARN_ENABLED;
  static const std::string CFG_KEY_ICONTHEME;
  static const std::string CFG_KEY_THEME_NAME;
  static const std::string CFG_KEY_THEME_DARK;
  static const std::string CFG_KEY_LIGHT_DARK_MODE;
  static const std::string CFG_KEY_FORCE_X11;

  static const std::string CFG_KEY_MAIN_WINDOW;
  static const std::string CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP;
  static const std::string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const std::string CFG_KEY_MAIN_WINDOW_X;
  static const std::string CFG_KEY_MAIN_WINDOW_Y;
  static const std::string CFG_KEY_MAIN_WINDOW_HEAD;

  static const std::string CFG_KEY_APPLET_FALLBACK_ENABLED;
  static const std::string CFG_KEY_APPLET_ICON_ENABLED;

  static const std::string CFG_KEY_TIMERBOX;
  static const std::string CFG_KEY_TIMERBOX_CYCLE_TIME;
  static const std::string CFG_KEY_TIMERBOX_POSITION;
  static const std::string CFG_KEY_TIMERBOX_FLAGS;
  static const std::string CFG_KEY_TIMERBOX_IMMINENT;
  static const std::string CFG_KEY_TIMERBOX_ENABLED;

private:
  static std::string get_break_name(workrave::BreakId id);
  static std::string expand(const std::string &str, workrave::BreakId id);

private:
  static inline std::shared_ptr<workrave::config::IConfigurator> config;
};

inline std::ostream &
operator<<(std::ostream &stream, BlockMode mode)
{
  switch (mode)
    {
    case BlockMode::Off:
      stream << "none";
      break;
    case BlockMode::Input:
      stream << "input";
      break;
    case BlockMode::All:
      stream << "all";
      break;
    }
  return stream;
}

inline std::ostream &
operator<<(std::ostream &stream, FocusMode mode)
{
  switch (mode)
    {
    case FocusMode::Suspended:
      stream << "suspended";
      break;
    case FocusMode::Quiet:
      stream << "quiet";
      break;
    }
  return stream;
}

#endif // WORKRAVE_UI_GUICONFIG_HH
