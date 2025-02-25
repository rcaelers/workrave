// Copyright (C) 2002, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <list>
#include <map>

#include <windows.h>

class Config
{
public:
  Config();

  bool get_value(const std::string &key, std::string &out) const;
  bool get_value(const std::string &key, int &out) const;
  bool get_value(const std::string &key, bool &out) const;

private:
  std::string key_windowsify(const std::string &key) const;
  std::string key_add_part(std::string s, std::string t) const;
  void key_split(const std::string &key, std::string &parent, std::string &child) const;

  void strip_trailing_slash(std::string &key) const;
  void add_trailing_slash(std::string &key) const;

  std::string key_root;
};

#endif // CONFIG_H
