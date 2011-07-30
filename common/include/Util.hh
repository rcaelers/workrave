// Util.hh --- General purpose utility functions
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2010 Rob Caelers & Raymond Penners
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

#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <set>

using namespace std;

class Util
{
public:
  // Grmbl. Ideally, this was call just SearchPath, however
  // Windows feels it is necessary to do a "#define SearchPath SearchPathA"
  enum SearchPathId
  {
    SEARCH_PATH_IMAGES = 0,
    SEARCH_PATH_SOUNDS,
    SEARCH_PATH_CONFIG,
    SEARCH_PATH_EXERCISES,
    SEARCH_PATH_SIZEOF
  };

  static const string& get_home_directory();
  static void set_home_directory(const string &home);

#ifdef PLATFORM_OS_WIN32
  static string get_application_directory();
  static bool registry_set_value(const char *path, const char *name, const char *value);
  static bool registry_get_value(const char *path, const char *name, char *out);
#endif
  static const set<string> &get_search_path(SearchPathId type);
  static bool file_exists(string path);
  static string complete_directory(string path, SearchPathId type);

  static bool running_gnome();

private:
  static set<string> search_paths[SEARCH_PATH_SIZEOF];
  static string home_directory;
};

#endif // UTIL_HH
