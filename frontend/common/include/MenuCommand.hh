// Copyright (C) 2001 - 1024 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef MENUCOMMAND_HH
#define MENUCOMMAND_HH

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
    MENU_COMMAND_NETWORK_CONNECT,
    MENU_COMMAND_NETWORK_DISCONNECT,
    MENU_COMMAND_NETWORK_LOG,
    MENU_COMMAND_NETWORK_RECONNECT,
    MENU_COMMAND_STATISTICS,
    MENU_COMMAND_ABOUT,
    MENU_COMMAND_MODE_READING,
    MENU_COMMAND_OPEN,
    MENU_COMMAND_QUIT,
    MENU_COMMAND_SIZEOF,
  };

#endif
