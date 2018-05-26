// Util.cc --- General purpose utility functions
//
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
#include "config.h"
#endif

#include <boost/filesystem.hpp>

#include "debug.hh"

#ifdef PLATFORM_OS_OSX
#include "OSXHelpers.hh"
#endif

#include <cstdlib>
#include <stdio.h>
#include <sstream>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef PLATFORM_OS_WIN32
#include <windows.h>
// HACK: #include <shlobj.h>, need -fvtable-thunks.
// Perhaps we should enable this, but let's hack it for now...
//#include <shlobj.h>
extern "C" {
#define SHGetPathFromIDList SHGetPathFromIDListA
HRESULT WINAPI SHGetSpecialFolderLocation(HWND,int,void**);
BOOL WINAPI SHGetPathFromIDList(void *,LPSTR);
#ifndef PLATFORM_OS_WIN32
VOID WINAPI CoTaskMemFree(PVOID);
#endif
#define PathCanonicalize PathCanonicalizeA
BOOL WINAPI PathCanonicalize(LPSTR,LPCSTR);
#define CSIDL_APPDATA   26
}
// (end of hack)
#endif

#include "utils/AssetPath.hh"
#include "utils/Platform.hh"

using namespace std;
using namespace workrave::utils;

set<string> AssetPath::search_paths[AssetPath::SEARCH_PATH_SIZEOF];
string AssetPath::home_directory = "";

//! Returns the user's home directory.
const string&
AssetPath::get_home_directory()
{
  // Already cached?
  static string ret;

  if (home_directory.length() != 0)
    {
      ret = home_directory;
    }
  else if (ret.length() == 0)
    {
      // Default to current directory
      ret = "./";

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_OSX)
      const char *home = getenv("WORKRAVE_HOME");

      if (home == nullptr)
        {
          home = getenv("HOME");
        }

      if (home != nullptr)
        {
          ret = home;
          ret += "/.workrave/";

          mkdir(ret.c_str(), 0700);
        }
#elif defined(PLATFORM_OS_WIN32)
      void *pidl;
      HRESULT hr = SHGetSpecialFolderLocation(HWND_DESKTOP,
                                              CSIDL_APPDATA, &pidl);
      if (SUCCEEDED(hr))
        {
          char buf[MAX_PATH];

          SHGetPathFromIDList(pidl, buf);
          CoTaskMemFree(pidl);

          strcat (buf, "\\Workrave");
          BOOL dirok = CreateDirectory(buf, NULL);
          if (! dirok)
            {
              if (GetLastError() == ERROR_ALREADY_EXISTS)
                {
                  dirok = TRUE;
                }
            }

          if (dirok)
            {
              ret = string(buf) + "\\";
            }
        }
#endif
    }

  return ret;
}


//! Returns the user's home directory.
void
AssetPath::set_home_directory(const string &home)
{
#ifdef PLATFORM_OS_WIN32
  if (home.substr(0, 2) == ".\\" ||
      home.substr(0, 3) == "..\\")
    {
      char buffer[MAX_PATH];

      // Path relative to location of workrave root.
      string appdir = Platform::get_application_directory();

      home_directory = appdir + "\\" + home + "\\";

      PathCanonicalize(buffer, home_directory.c_str());
      home_directory = buffer;
    }
  else
#endif
    {
      home_directory = home + "/";
    }

#ifdef PLATFORM_OS_WIN32
  CreateDirectory(home_directory.c_str(), NULL);
#else
  mkdir(home_directory.c_str(), 0777);
#endif
}

#ifdef PLATFORM_OS_WIN32

#endif


//! Returns the searchpath for the specified file type.
const set<string> &
AssetPath::get_search_path(SearchPathId type)
{
  if (search_paths[type].size() > 0)
    return search_paths[type];

  set<string> &searchPath = search_paths[type];

  string home_dir = get_home_directory();
#if defined(PLATFORM_OS_WIN32)
  string app_dir = Platform::get_application_directory();
#elif defined(PLATFORM_OS_OSX)
  char execpath[MAXPATHLEN+1];
  uint32_t pathsz = sizeof (execpath);

  _NSGetExecutablePath (execpath, &pathsz);

  boost::filesystem::path p(execpath);
  boost::filesystem::path dir = p.parent_path();
  string app_dir = dir.string();
#endif

  if (type == SEARCH_PATH_IMAGES)
    {
#if defined(PLATFORM_OS_UNIX)
      if (home_dir != "./")
        {
          searchPath.insert(home_dir + "/images");
        }
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/images");
      searchPath.insert(string(WORKRAVE_DATADIR) + "/icons/hicolor");
      searchPath.insert("/usr/local/share/workrave/images");
      searchPath.insert("/usr/share/workrave/images");
#elif defined(PLATFORM_OS_WIN32)
      searchPath.insert(app_dir + "\\share\\images");

#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/images");
      searchPath.insert(app_dir +  "/../Resources/images");
#endif
    }
  if (type == SEARCH_PATH_SOUNDS)
    {
#if defined(PLATFORM_OS_UNIX)
      if (home_dir != "./")
        {
          searchPath.insert(home_dir + "/sounds");
        }
      searchPath.insert(string(WORKRAVE_DATADIR) + "/sounds/workrave");
      searchPath.insert("/usr/local/share/sounds/workrave");
      searchPath.insert("/usr/share/sounds/workrave");
#elif defined(PLATFORM_OS_WIN32)
      searchPath.insert(app_dir + "\\share\\sounds");
#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_DATADIR) + "/sounds");
      searchPath.insert(app_dir +  "/../Resources/sounds");
#endif
    }
  else if (type == SEARCH_PATH_CONFIG)
    {
#if defined(PLATFORM_OS_UNIX)
      if (home_dir != "./")
        {
          searchPath.insert(home_dir + "/");
          searchPath.insert(home_dir + "/etc");
        }
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/etc");
      searchPath.insert("/usr/local/share/workrave/etc");
      searchPath.insert("/usr/share/workrave/etc");
#elif defined(PLATFORM_OS_WIN32)
      searchPath.insert(home_dir + "\\");
      searchPath.insert(app_dir + "\\etc");
#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/etc");
      searchPath.insert(home_dir + "/");
      searchPath.insert(app_dir +  "/../Resources/config");
#endif
    }
  else if (type == SEARCH_PATH_EXERCISES)
    {
#if defined(PLATFORM_OS_UNIX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/exercises");
#elif defined(PLATFORM_OS_WIN32)
      searchPath.insert(app_dir + "\\share\\exercises");
#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/exercises");
      searchPath.insert(app_dir +  "/../Resources/exercises");
#else
#error Not properly ported.
#endif
    }

  return searchPath;
}


//! Completes the directory for the specified file and file type.
string
AssetPath::complete_directory(string path, AssetPath::SearchPathId type)
{
  boost::filesystem::path full_path;
  bool found = false;

  const set<string> &search_path = get_search_path(type);

  for (set<string>::const_iterator i = search_path.begin(); !found && i != search_path.end(); ++i)
    {
      full_path = (*i);
      full_path /= path;
      found = boost::filesystem::is_regular_file(full_path);
    }

  if (!found)
    {
      full_path = path;
    }

  return full_path.string();
}

;
