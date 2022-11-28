// Copyright (C) 2001 - 2011, 2013 Rob Caelers & Raymond Penners
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
#  include "config.h"
#endif

#include <filesystem>

#include "debug.hh"

#include "utils/AssetPath.hh"
#include "utils/Platform.hh"
#include "utils/Paths.hh"

using namespace workrave::utils;

std::list<std::filesystem::path> AssetPath::search_paths[AssetPath::SEARCH_PATH_SIZEOF];

//! Returns the search_path for the specified file type.
const std::list<std::filesystem::path> &
AssetPath::get_search_path(SearchPathId type)
{
  TRACE_ENTRY();
  if (!search_paths[type].empty())
    {
      return search_paths[type];
    }

  std::list<std::filesystem::path> &search_path = search_paths[type];

  std::list<std::filesystem::path> data_directories = Paths::get_data_directories();
  std::list<std::filesystem::path> config_directories = Paths::get_config_directories();

  if (type == SEARCH_PATH_IMAGES)
    {
      for (const auto &directory: data_directories)
        {
#if defined(PLATFORM_OS_UNIX)
          search_path.push_back(directory / "workrave/images");
          search_path.push_back(directory / "icons/hicolor");
#elif defined(PLATFORM_OS_MACOS)
          search_path.push_back(directory / "images");
#elif defined(PLATFORM_OS_WINDOWS)
          search_path.push_back(directory / "icons");
          search_path.push_back(directory / "images");
#endif
        }
    }
  if (type == SEARCH_PATH_SOUNDS)
    {
      for (const auto &directory: data_directories)
        {
#if defined(PLATFORM_OS_UNIX)
          search_path.push_back(directory / "sounds/workrave");
#elif defined(PLATFORM_OS_WINDOWS)
          search_path.push_back(directory / "sounds");
#elif defined(PLATFORM_OS_MACOS)
          search_path.push_back(directory / "sounds");
#endif
        }
    }
  else if (type == SEARCH_PATH_CONFIG)
    {
      for (const auto &directory: config_directories)
        {
          search_path.push_back(directory);
        }
    }
  else if (type == SEARCH_PATH_EXERCISES)
    {
      for (const auto &directory: data_directories)
        {
#if defined(PLATFORM_OS_UNIX)
          search_path.push_back(directory / "workrave/exercises");
#elif defined(PLATFORM_OS_WINDOWS)
          search_path.push_back(directory / "exercises");
#elif defined(PLATFORM_OS_MACOS)
          search_path.push_back(directory / "exercises");
#endif
        }
    }

#if defined(HAVE_TRACING)
  TRACE_MSG("Search path for {}", type);
  for (const auto &d: search_path)
    {
      TRACE_VAR(d);
    }
#endif

  return search_path;
}

std::string
AssetPath::complete_directory(std::string path, AssetPath::SearchPathId type)
{
  std::filesystem::path full_path;
  bool found = false;

  const std::list<std::filesystem::path> &search_path = get_search_path(type);

  for (auto i = search_path.begin(); !found && i != search_path.end(); ++i)
    {
      full_path = (*i);
      full_path /= path;
      found = std::filesystem::is_regular_file(full_path);
    }

  if (!found)
    {
      full_path = path;
    }

  return full_path.string();
}

bool
AssetPath::complete_directory(std::string path, AssetPath::SearchPathId type, std::string &complete_path)
{
  bool found = false;

  const std::list<std::filesystem::path> &search_path = get_search_path(type);

  for (auto i = search_path.begin(); !found && i != search_path.end(); ++i)
    {
      std::filesystem::path full_path;
      full_path = (*i);
      full_path /= path;
      found = std::filesystem::is_regular_file(full_path);
      complete_path = full_path.string();
    }

  return found;
}
