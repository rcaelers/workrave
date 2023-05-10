// Copyright (C) 2001 - 2021 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_COMMON_MENUDEFS_HH
#define WORKRAVE_UI_COMMON_MENUDEFS_HH

#if __cplusplus > 201703L
#  include <string_view>
#endif

enum MenuCommand
{
  // Note: Do NOT remove/change any of the commands.
  // Append new items only at the end.
  MENU_COMMAND_PREFERENCES,
  MENU_COMMAND_EXERCISES,
  MENU_COMMAND_REST_BREAK,
  MENU_COMMAND_MODE_NORMAL,
  MENU_COMMAND_MODE_QUIET,
  MENU_COMMAND_MODE_SUSPENDED,
  MENU_COMMAND_REMOVED_1,
  MENU_COMMAND_REMOVED_2,
  MENU_COMMAND_REMOVED_3,
  MENU_COMMAND_REMOVED_4,
  MENU_COMMAND_STATISTICS,
  MENU_COMMAND_ABOUT,
  MENU_COMMAND_MODE_READING,
  MENU_COMMAND_OPEN,
  MENU_COMMAND_QUIT,
  MENU_COMMAND_REMOVED_5,
  MENU_COMMAND_MODE_SUBMENU,
  MENU_COMMAND_MODE,
  MENU_COMMAND_SIZEOF,
};

#if __cplusplus > 201703L
class MenuId
{
public:
  using sv = std::string_view;
  static constexpr std::string_view PREFERENCES = sv("workrave.preferences");
  static constexpr std::string_view EXERCISES = sv("workrave.exercises");
  static constexpr std::string_view REST_BREAK = sv("workrave.restbreak");
  static constexpr std::string_view MODE_MENU = sv("workrave.mode_menu");
  static constexpr std::string_view MODE = sv("workrave.mode");
  static constexpr std::string_view MODE_NORMAL = sv("workrave.mode_normal");
  static constexpr std::string_view MODE_QUIET = sv("workrave.mode_quiet");
  static constexpr std::string_view MODE_SUSPENDED = sv("workrave.mode_suspended");
  static constexpr std::string_view STATISTICS = sv("workrave.statistics");
  static constexpr std::string_view ABOUT = sv("workrave.about");
  static constexpr std::string_view MODE_READING = sv("workrave.mode_reading");
  static constexpr std::string_view OPEN = sv("workrave.open");
  static constexpr std::string_view QUIT = sv("workrave.quit");
};
#endif

#if __cplusplus > 201703L
enum class MenuAction
{
  Preferences = MENU_COMMAND_PREFERENCES,
  Exercises = MENU_COMMAND_EXERCISES,
  Restbreak = MENU_COMMAND_REST_BREAK,
  ModeNormal = MENU_COMMAND_MODE_NORMAL,
  ModeQuiet = MENU_COMMAND_MODE_QUIET,
  ModeSuspended = MENU_COMMAND_MODE_SUSPENDED,
  Statistics = MENU_COMMAND_STATISTICS,
  About = MENU_COMMAND_ABOUT,
  ModeReading = MENU_COMMAND_MODE_READING,
  Open = MENU_COMMAND_OPEN,
  Quiet = MENU_COMMAND_QUIT,
  ModeMenu = MENU_COMMAND_MODE_SUBMENU,
  Mode = MENU_COMMAND_MODE,
  Size = MENU_COMMAND_SIZEOF,
};
#endif

#if __cplusplus > 201703L
enum
#else
enum MenuItemType
#endif
{
  MENU_ITEM_TYPE_SUBMENU_BEGIN = 1,
  MENU_ITEM_TYPE_SUBMENU_END = 2,
  MENU_ITEM_TYPE_RADIOGROUP_BEGIN = 3,
  MENU_ITEM_TYPE_RADIOGROUP_END = 4,
  MENU_ITEM_TYPE_ACTION = 5,
  MENU_ITEM_TYPE_CHECK = 6,
  MENU_ITEM_TYPE_RADIO = 7,
  MENU_ITEM_TYPE_SEPARATOR = 8,
};

#if __cplusplus > 201703L
enum class MenuItemType
{
  SubMenuBegin = MENU_ITEM_TYPE_SUBMENU_BEGIN,
  SubMenuEnd = MENU_ITEM_TYPE_SUBMENU_END,
  RadioGroupBegin = MENU_ITEM_TYPE_RADIOGROUP_BEGIN,
  RadioGroupEnd = MENU_ITEM_TYPE_RADIOGROUP_END,
  Action = MENU_ITEM_TYPE_ACTION,
  Check = MENU_ITEM_TYPE_CHECK,
  Radio = MENU_ITEM_TYPE_RADIO,
  Separator = MENU_ITEM_TYPE_SEPARATOR,
};
#endif

enum MenuItemFlag
{
  MENU_ITEM_FLAG_NONE = 0,
  MENU_ITEM_FLAG_ACTIVE = 1,
  MENU_ITEM_FLAG_VISIBLE = 2,
};

#endif // WORKRAVE_UI_COMMON_MENUDEFS_HH
