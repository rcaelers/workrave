// StringUtil.cc --- General purpose string utility functions
//
// Copyright (C) 2007, 2011 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <cstdlib>
#include <stdio.h>
#include <sstream>

#include "StringUtil.hh"

using namespace std;

void
StringUtil::split(const string &in, const char delim, vector<std::string> &result)
{
  string::size_type start_pos = 0;
  string::size_type end_pos = in.find(delim, 0);

  while ((end_pos = in.find(delim, start_pos)) != string::npos)
    {
      string s = in.substr(start_pos, end_pos - start_pos);
      result.push_back(s);

      start_pos = end_pos + 1;
      end_pos = in.find(delim, start_pos);
    }

  string s = in.substr(start_pos, in.size() - start_pos);
  result.push_back(s);
}


string
StringUtil::search_replace(const string &in, const string &search, const string &replace)
{
  string str = in;
  string::size_type pos = 0;

  while ((pos = str.find(search, pos)) != string::npos)
    {
      str.replace(pos, search.size(), replace);
      pos++;
    }

  return str;
}
