// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKAVE_LIBS_UTILS_PATHS_HH
#define WORKAVE_LIBS_UTILS_PATHS_HH

#include <string>
#include <list>
#include <filesystem>

namespace workrave::utils
{
  class Paths
  {
  public:
    static std::filesystem::path get_home_directory();
    static std::filesystem::path get_application_directory();
    static std::list<std::filesystem::path> get_data_directories();
    static std::list<std::filesystem::path> get_config_directories();
    static std::list<std::filesystem::path> get_state_directories();
    static std::filesystem::path get_config_directory();
    static std::filesystem::path get_state_directory();
    static void set_portable_directory(const std::string &new_config_directory);
    static std::filesystem::path get_log_directory();

  private:
    static std::list<std::filesystem::path> canonicalize(std::list<std::filesystem::path> paths);
  };
} // namespace workrave::utils

#endif // WORKAVE_LIBS_UTILS_PATHS_HH
