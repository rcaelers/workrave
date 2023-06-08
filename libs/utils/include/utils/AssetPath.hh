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
#include <list>
#include <filesystem>

#include "utils/Enum.hh"

namespace workrave::utils
{
  // Grmbl. Ideally, this was call just SearchPath, however
  // Windows feels it is necessary to do a "#define SearchPath SearchPathA"
  enum SearchPathId
  {
    Images,
    Sounds,
    Config,
    Exercises,
  };

  template<>
  struct enum_traits<SearchPathId>
  {
    static constexpr auto min = SearchPathId::Images;
    static constexpr auto max = SearchPathId::Exercises;
    static constexpr auto linear = true;

    static constexpr std::array<std::pair<std::string_view, SearchPathId>, 4> names{{{"images", SearchPathId::Images},
                                                                                     {"sounds", SearchPathId::Sounds},
                                                                                     {"config", SearchPathId::Config},
                                                                                     {"exercises", SearchPathId::Exercises}}};
  };

  class AssetPath
  {
  public:
    static const std::list<std::filesystem::path> &get_search_path(SearchPathId type);
    static std::string complete_directory(std::string path, SearchPathId type);
    static bool complete_directory(std::string path, SearchPathId type, std::string &full_path);

  private:
    static std::list<std::filesystem::path> search_paths[workrave::utils::enum_count<SearchPathId>()];
  };
} // namespace workrave::utils

#endif // WORKRAVE_UTILS_UTIL_HH
