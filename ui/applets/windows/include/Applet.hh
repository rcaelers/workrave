// Copyright (C) 2001 - 2007 Rob Caelers & Raymond Penners
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

#ifndef APPLET_H
#define APPLET_H

#include <cstdint>
#include <string>
#include <list>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>

#include "core/CoreTypes.hh"
#include "commonui/MenuDefs.hh"

#define APPLET_WINDOW_CLASS_NAME "WorkraveApplet"
#define APPLET_BAR_TEXT_MAX_LENGTH 16

#define APPLET_MESSAGE_HEARTBEAT 0
#define APPLET_MESSAGE_MENU 1

/*!!!!!!!!!!!!!!!!!!!
On Windows x64, the installed Applet will be 64-bit.

Therefore Workrave (32-bit) could be passing structures to a
64-bit applet. And so we must ensure that all members of any
passed structure have types that are the same size on both
32 and 64 bit Windows.

All structures declared in this file are used by both
Workrave (x86) and the applet (x86 & x64).
*/

struct AppletHeartbeatData
{
  volatile bool enabled;
  short slots[workrave::BREAK_ID_SIZEOF];

  char bar_text[workrave::BREAK_ID_SIZEOF][APPLET_BAR_TEXT_MAX_LENGTH];

  short bar_secondary_color[workrave::BREAK_ID_SIZEOF];
  int bar_secondary_val[workrave::BREAK_ID_SIZEOF];
  int bar_secondary_max[workrave::BREAK_ID_SIZEOF];

  short bar_primary_color[workrave::BREAK_ID_SIZEOF];
  int bar_primary_val[workrave::BREAK_ID_SIZEOF];
  int bar_primary_max[workrave::BREAK_ID_SIZEOF];
};

struct TimerData
{
  std::string bar_text;
  int slot;
  uint32_t bar_secondary_color;
  uint32_t bar_secondary_val;
  uint32_t bar_secondary_max;
  uint32_t bar_primary_color;
  uint32_t bar_primary_val;
  uint32_t bar_primary_max;

private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    ar &bar_text &slot &bar_secondary_color &bar_secondary_val &bar_secondary_max &bar_primary_color &bar_primary_val
      &bar_primary_max;
  }
};

struct AppletMenuItem
{
  AppletMenuItem() = default;
  AppletMenuItem(std::string text,
                 std::string dynamic_text,
                 std::string action,
                 uint32_t command,
                 MenuItemType type,
                 uint8_t flags = 0)
    : text(std::move(text))
    , dynamic_text(std::move(dynamic_text))
    , action(std::move(action))
    , command(command)
    , type(static_cast<std::underlying_type_t<MenuItemType>>(type))
    , flags(flags)
  {
  }
  AppletMenuItem(std::string text,
                 std::string dynamic_text,
                 std::string action,
                 uint32_t command,
                 uint8_t type,
                 uint8_t flags = 0)
    : text(std::move(text))
    , dynamic_text(std::move(dynamic_text))
    , action(std::move(action))
    , command(command)
    , type(type)
    , flags(flags)
  {
  }
  std::string text;
  std::string dynamic_text;
  std::string action;
  uint32_t command;
  uint8_t type;
  uint8_t flags;

private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    ar &text &dynamic_text &action &command &type &flags;
  }
};

struct AppletMenuData
{
  /*
  MSDN notes:
  Handles have 32 significant bits on 64-bit Windows
  .
  We must ensure that our types are the same size
  on both 64 and 32 bit systems (see x64 comment).

  We will pass the command_window HWND as a int64_t:
  */
  int64_t command_window;
  std::list<AppletMenuItem> items;

private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive &ar, const unsigned int version)
  {
    ar &command_window &items;
  }
};

#endif /* APPLET_H */
