// StringUtil.hh --- General purpose string utility functions
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

#ifndef STRINGUTIL_HH
#define STRINGUTIL_HH

#include <string>
#include <vector>

class StringUtil
{
public:
  static void split(const std::string &in, const char delim, std::vector<std::string> &result);
  static std::string search_replace(const std::string &in, const std::string &search, const std::string &replace);
};

#endif // STRINGUTIL_HH
