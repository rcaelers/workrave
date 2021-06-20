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
#  define GUICONFIG_HH

#  include "core/ICore.hh"
#  include "config/Setting.hh"

class GUIConfig
{
public:
  enum BlockMode
  {
    BLOCK_MODE_NONE = 0,
    BLOCK_MODE_INPUT,
    BLOCK_MODE_ALL
  };

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
  static workrave::config::Setting<int, GUIConfig::BlockMode> &block_mode();
  static workrave::config::Setting<std::string> &locale();
  static workrave::config::Setting<bool> &trayicon_enabled();
  static workrave::config::Setting<bool> &closewarn_enabled();
  static workrave::config::Setting<bool> &autostart_enabled();
  static workrave::config::Setting<std::string> &icon_theme();

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

  static void init();

private:
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
  static std::string expand(const std::string &str, workrave::BreakId id);
};

#endif

// Backend::get_configurator()->get_value_with_default(
//   "advanced/force_focus_on_break_start",
//   force_focus_on_break_start,
//   true
//   );

//         Backend::get_configurator()->get_value(SoundTheme::CFG_KEY_SOUND_MUTE, mute);

// Backend::get_configurator()->get_value_with_default("advanced/monitor",
//                                                         monitor_type,
//                                                         "default");
//     bool valid = Backend::get_configurator()->get_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
//                                                             row[sound_model.label],
//                                                             filename);
//   // Should SetWindowOnTop() call IMEWindowMagic() ?
//   if( !Backend::get_configurator()->get_value( "advanced/ime_magic", ime_magic ) )

//   if( !Backend::get_configurator()->get_value( "advanced/reset_window_always", reset_window_always ) )

//   if( !Backend::get_configurator()->get_value( "advanced/reset_window_never", reset_window_never ) )

//   if( Backend::get_configurator()->get_value( "advanced/force_focus_functions", str ) )
