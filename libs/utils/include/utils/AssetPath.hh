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

#ifndef WORKRAVE_UTILS_UTIL_HH
#define WORKRAVE_UTILS_UTIL_HH

#include <string>
#include <set>

using namespace std;

namespace workrave
{
  namespace utils
  {
    class AssetPath
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

      static const set<string> &get_search_path(SearchPathId type);
      static string complete_directory(string path, SearchPathId type);

    private:
      static set<string> search_paths[SEARCH_PATH_SIZEOF];
      static string home_directory;
    };
  }
}

#endif // WORKRAVE_UTILS_UTIL_HH
