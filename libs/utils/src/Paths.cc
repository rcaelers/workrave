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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <iostream>
#include <algorithm>

#include "utils/Paths.hh"

#include "debug.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <shlobj.h>
#  include <shlwapi.h>
#  include <windows.h>
#endif

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSHelpers.hh"
#endif

#if defined HAVE_GLIB
#  include <glib.h>
#endif

using namespace workrave::utils;

#if defined(HAVE_GLIB)
#  include <glib.h>
#endif

namespace
{
  static std::filesystem::path portable_directory;

#if defined(PLATFORM_OS_WINDOWS)
  std::filesystem::path get_special_folder(REFKNOWNFOLDERID folder)
  {
    PWSTR path = nullptr;
    auto hr = ::SHGetKnownFolderPath(folder, KF_FLAG_DEFAULT, nullptr, &path);
    if (SUCCEEDED(hr))
      {
        auto p = std::filesystem::path(path);
        ::CoTaskMemFree(path);
        return std::filesystem::canonical(p);
      }
    return {};
  }
#endif
#if defined(PLATFORM_OS_UNIX)
  std::filesystem::path get_directory_from_environment(const char *env)
  {
    auto dir = std::getenv(env);
    if (dir != nullptr)
      {
        return dir;
      }

    return {};
  }
#endif
} // namespace

void
Paths::set_portable_directory(const std::string &new_portable_directory)
{
  TRACE_ENTRY();
  try
    {
      std::filesystem::path directory(new_portable_directory);

      if (directory.is_relative())
        {
#if defined(PLATFORM_OS_WINDOWS)
          directory = Paths::get_application_directory() / directory;
#else
          directory = std::filesystem::absolute(directory);
#endif
        }

      portable_directory = std::filesystem::weakly_canonical(directory);

      std::filesystem::create_directories(portable_directory);
      std::filesystem::permissions(portable_directory,
                                   std::filesystem::perms::others_all | std::filesystem::perms::group_all,
                                   std::filesystem::perm_options::remove);
    }
  catch (std::exception &e)
    {
      TRACE_VAR(e.what());
      std::cout << e.what() << "\n";
    }
}

std::filesystem::path
Paths::get_home_directory()
{
  TRACE_ENTRY();
  std::filesystem::path ret;
  try
    {
      ret = std::filesystem::current_path();

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_MACOS)
      const char *home = getenv("WORKRAVE_HOME");
      if (home == nullptr)
        {
          home = getenv("HOME");
        }

      if (home != nullptr)
        {
          ret = home;
        }
#elif defined(PLATFORM_OS_WINDOWS)
      ret = get_special_folder(FOLDERID_RoamingAppData);
#endif
    }
  catch (std::exception &e)
    {
      TRACE_VAR(e.what());
      std::cout << e.what() << "\n";
    }
  return ret;
}

std::filesystem::path
Paths::get_application_directory()
{
  TRACE_ENTRY();
  try
    {
#if defined(PLATFORM_OS_WINDOWS)
      char app_dir_name[MAX_PATH];
      GetModuleFileName(GetModuleHandle(NULL), app_dir_name, sizeof(app_dir_name));
      // app_dir_name == c:\program files\workrave\lib\workrave.exe
      char *s = strrchr(app_dir_name, '\\');
      assert(s);
      *s = '\0';
      // app_dir_name == c:\program files\workrave\lib
      s = strrchr(app_dir_name, '\\');
      assert(s);
      *s = '\0';
      // app_dir_name == c:\program files\workrave
      return std::filesystem::path(app_dir_name);
#elif defined(PLATFORM_OS_MACOS)
      char execpath[MAXPATHLEN + 1];
      uint32_t pathsz = sizeof(execpath);

      _NSGetExecutablePath(execpath, &pathsz);

      std::filesystem::path p(execpath);
      std::filesystem::path dir = p.parent_path().parent_path();
      TRACE_VAR(dir);
      return dir;
#endif
    }
  catch (std::exception &e)
    {
    }

  return std::filesystem::path();
}

std::list<std::filesystem::path>
Paths::get_data_directories()
{
  TRACE_ENTRY();
  std::list<std::filesystem::path> directories;

  try
    {
#if defined(PLATFORM_OS_UNIX)
      directories.push_back(WORKRAVE_DATADIR "/");
      directories.push_back("/usr/local/share/");
      directories.push_back("/usr/share/");
#endif

#if defined(HAVE_GLIB)
      const gchar *user_data_dir = g_get_user_data_dir();
      directories.push_back(std::filesystem::path(user_data_dir) / "workrave");

      const char *const *system_data_dirs = g_get_system_data_dirs();
      for (int i = 0; system_data_dirs && system_data_dirs[i]; ++i)
        {
          if (system_data_dirs[i][0] != '\0')
            {
              directories.emplace_back(system_data_dirs[i]);
            }
        }
#endif

#if defined(PLATFORM_OS_WINDOWS)
      directories.push_back(get_application_directory() / "share");
#elif defined(PLATFORM_OS_MACOS)
      directories.push_back(get_application_directory() / "Resources" / "share");
#endif
    }
  catch (std::exception &e)
    {
      TRACE_VAR(e.what());
    }

#if defined(HAVE_TRACING)
  for (const auto &d: canonicalize(directories))
    {
      TRACE_VAR(d.string());
    }
#endif

  return canonicalize(directories);
}

std::list<std::filesystem::path>
Paths::get_config_directories()
{
  TRACE_ENTRY();
  std::list<std::filesystem::path> directories;
  try
    {
#if defined(PLATFORM_OS_WINDOWS)
      directories.push_back(get_application_directory() / "etc");
      directories.push_back(get_home_directory() / "Workrave");
#endif

#if defined(HAVE_GLIB)
      const gchar *user_config_dir = g_get_user_config_dir();
      directories.push_back(std::filesystem::path(user_config_dir) / "workrave");
#endif

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_MACOS)
      directories.push_back(get_home_directory() / ".workrave");
#endif
    }
  catch (std::exception &e)
    {
      TRACE_VAR(e.what());
    }

#if defined(HAVE_TRACING)
  for (const auto &d: canonicalize(directories))
    {
      TRACE_VAR(d.string());
    }
#endif
  return canonicalize(directories);
}

std::filesystem::path
Paths::get_config_directory()
{
  TRACE_ENTRY();
  std::filesystem::path ret;

  try
    {
      if (!portable_directory.empty())
        {
          ret = portable_directory / "etc";
          TRACE_MSG("Using portable config directory");
        }
      else
        {
          std::list<std::filesystem::path> directories = get_config_directories();
          auto it = std::find_if(directories.begin(), directories.end(), [](const auto &d) {
            return std::filesystem::is_directory(d);
          });
          if (it == directories.end())
            {
              TRACE_MSG("Using preferred directory");
              ret = directories.front();
            }
          else
            {
              TRACE_MSG("Using existing directory");
              ret = *it;
            }
        }

      if (!std::filesystem::is_directory(ret))
        {
          TRACE_MSG("Creating home directory");
          std::filesystem::create_directories(ret);
          std::filesystem::permissions(ret,
                                       std::filesystem::perms::others_all | std::filesystem::perms::group_all,
                                       std::filesystem::perm_options::remove);
        }
    }
  catch (std::exception &e)
    {
      TRACE_MSG("Exception: {}", e.what());
    }

  TRACE_VAR(ret);
  return ret;
}

std::filesystem::path
Paths::get_state_directory()
{
  TRACE_ENTRY();
  std::filesystem::path ret;

  try
    {
      if (!portable_directory.empty())
        {
          ret = portable_directory;
          TRACE_MSG("Using portable config directory");
        }
      else
        {
          std::list<std::filesystem::path> directories = get_config_directories();
          auto it = std::find_if(directories.begin(), directories.end(), [](const auto &d) {
            return std::filesystem::is_regular_file(d / "state");
          });
          if (it != directories.end())
            {
              TRACE_MSG("Using existing directory");
              ret = *it;
            }

          if (ret.empty())
            {
              TRACE_MSG("Using preferred directory");
#if defined(PLATFORM_OS_WINDOWS)
              ret = get_home_directory() / "Workrave";
#elif defined(HAVE_GLIB)
              const gchar *user_data_dir = g_get_user_data_dir();
              ret = std::filesystem::path(user_data_dir) / "workrave";
#else
              ret = get_home_directory() / ".workrave";
#endif
            }

          if (ret.empty())
            {
              TRACE_MSG("Using config directory");
              ret = get_config_directory();
            }
        }

      if (!std::filesystem::is_directory(ret))
        {
          TRACE_MSG("Creating home directory");
          std::filesystem::create_directories(ret);
          std::filesystem::permissions(ret,
                                       std::filesystem::perms::others_all | std::filesystem::perms::group_all,
                                       std::filesystem::perm_options::remove);
        }
    }
  catch (std::exception &e)
    {
      TRACE_MSG("Exception: {}", e.what());
    }

  TRACE_VAR(ret);
  return ret;
}

std::list<std::filesystem::path>
Paths::canonicalize(std::list<std::filesystem::path> paths)
{
  std::list<std::filesystem::path> ret;
  for (const auto &path: paths)
    {
      auto canonical_path = std::filesystem::weakly_canonical(path);
      if (find(ret.begin(), ret.end(), canonical_path) == ret.end())
        {
          ret.push_back(canonical_path);
        }
    }
  return ret;
}

#if defined(PLATFORM_OS_UNIX)
std::filesystem::path
Paths::get_log_directory()
{
  std::filesystem::path dir;

  dir = get_directory_from_environment("XDG_STATE_HOME");

  if (dir.empty())
    {
      dir = get_directory_from_environment("XDG_CACHE_HOME");
    }

  if (dir.empty())
    {
      dir = get_home_directory();
      if (!dir.empty())
        {
          dir /= ".cache";
        }
    }

  if (dir.empty())
    {
      return "/tmp";
    }

  return dir / "workrave";
}

#elif defined(PLATFORM_OS_MACOS)

std::filesystem::path
Paths::get_log_directory()
{
  std::filesystem::path dir = get_home_directory();
  if (!dir.empty())
    {
      dir /= std::filesystem::path("Library") / "Logs" / "Workrave";
      return dir;
    }

  return "/tmp";
}

#elif defined(PLATFORM_OS_WINDOWS)

std::filesystem::path
Paths::get_log_directory()
{
  std::filesystem::path dir = get_special_folder(FOLDERID_LocalAppData);
  if (!dir.empty())
    {
      dir /= std::filesystem::path("Workrave") / "Logs";
      return dir;
    }

  return "/tmp";
}

#endif
