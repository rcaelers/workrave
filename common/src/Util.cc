// Util.cc --- General purpose utility functions
//
// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#ifdef PLATFORM_OS_OSX
#include <mach-o/dyld.h>
#include <sys/param.h>
#endif

#include "Util.hh"

#include <glib.h>

using namespace std;

set<string> Util::search_paths[Util::SEARCH_PATH_SIZEOF];
string Util::home_directory = "";

//! Returns the user's home directory.
const string&
Util::get_home_directory()
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

      if (home == NULL)
        {
          home = getenv("HOME");
        }

      if (home != NULL)
        {
          ret = home;
          ret += "/.workrave/";

          mkdir(ret.c_str(), 0700);
        }
#elif defined(PLATFORM_OS_WIN32)
      void *pidl;
      char buf[MAX_PATH];
      HRESULT hr = SHGetSpecialFolderLocation(HWND_DESKTOP,
                                              CSIDL_APPDATA, &pidl);
      if (SUCCEEDED(hr))
        {
          SHGetPathFromIDList(pidl, buf);
          CoTaskMemFree(pidl);

          strcat (buf, "\\Workrave");
          BOOL dirok = FALSE;
          dirok = CreateDirectory(buf, NULL);
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
Util::set_home_directory(const string &home)
{
#ifdef PLATFORM_OS_WIN32
  if (home.substr(0, 2) == ".\\" ||
      home.substr(0, 3) == "..\\")
    {
      char buffer[MAX_PATH];

      // Path relative to location of workrave root.
      string appdir = get_application_directory();

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

//! Returns \c true if the specified file exists.
bool
Util::file_exists(string path)
{
  // 'stat' might be faster. but this is portable..
  FILE *f = NULL;
  bool ret = false;

  f = fopen(path.c_str(), "r");
  if (f != NULL)
    {
      fclose(f);
      ret = true;
    }

  return ret;
}


#ifdef PLATFORM_OS_WIN32
//! Returns the directory in which workrave is installed.
string
Util::get_application_directory()
{
  char app_dir_name[MAX_PATH];
  GetModuleFileName(GetModuleHandle(NULL), app_dir_name, sizeof(app_dir_name));
  // app_dir_name == c:\program files\workrave\lib\workrave.exe
  char *s = strrchr(app_dir_name, '\\');
  assert (s);
  *s = '\0';
  // app_dir_name == c:\program files\workrave\lib
  s = strrchr(app_dir_name, '\\');
  assert (s);
  *s = '\0';
  // app_dir_name == c:\program files\workrave
  return string(app_dir_name);
}

bool
Util::registry_get_value(const char *path, const char *name,
                         char *out)
{
  HKEY handle;
  bool rc = false;
  LONG err;

  err = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      size = MAX_PATH;
      err = RegQueryValueExA(handle, name, 0, &type, (LPBYTE) out, &size);
      if (err == ERROR_SUCCESS)
        {
          rc = true;
        }
      RegCloseKey(handle);
    }
  return rc;
}

bool
Util::registry_set_value(const char *path, const char *name,
                         const char *value)
{
  HKEY handle;
  bool rc = false;
  DWORD disp;
  LONG err;

  err = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      if (value != NULL)
        {
          err = RegSetValueEx(handle, name, 0, REG_SZ, (BYTE *) value,
                              strlen(value)+1);
        }
      else
        {
          err = RegDeleteValue(handle, name);
        }
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  return rc;
}

#endif


//! Returns the searchpath for the specified file type.
const set<string> &
Util::get_search_path(SearchPathId type)
{
  if (search_paths[type].size() > 0)
    return search_paths[type];

  set<string> &searchPath = search_paths[type];

  string home_dir = get_home_directory();
#if defined(PLATFORM_OS_WIN32)
  string app_dir = get_application_directory();
#elif defined(PLATFORM_OS_OSX)
  char execpath[MAXPATHLEN+1];
  uint32_t pathsz = sizeof (execpath);

  _NSGetExecutablePath (execpath, &pathsz);

  gchar *dir_path = g_path_get_dirname(execpath);
  string app_dir = dir_path;
  g_free(dir_path);
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
#if defined(DATA_PATH)
 	    searchPath.insert(string(DATA_PATH) + "frontend\\common\\share\\images");
#endif
      searchPath.insert(app_dir + "\\share\\images");

#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/images");
      searchPath.insert(app_dir + "/share/workrave/images");
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
#if defined(DATA_PATH)
	  searchPath.insert(string(DATA_PATH) + "frontend\\common\\share\\sounds");
#endif
      searchPath.insert(app_dir + "\\share\\sounds");
#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_DATADIR) + "/sounds/workrave");
      searchPath.insert(app_dir + "/share/sounds/workrave");
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
      searchPath.insert(app_dir + "/etc");
      searchPath.insert(home_dir + "/");
      searchPath.insert(app_dir +  "/../Resources/config");
#endif
    }
  else if (type == SEARCH_PATH_EXERCISES)
    {
#if defined(PLATFORM_OS_UNIX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/exercises");
#elif defined(PLATFORM_OS_WIN32)
#if defined(DATA_PATH)
	  searchPath.insert(string(DATA_PATH) + "frontend\\plugin\\exercises\\common\\share");
#endif
      searchPath.insert(app_dir + "\\share\\exercises");
#elif defined(PLATFORM_OS_OSX)
      searchPath.insert(string(WORKRAVE_PKGDATADIR) + "/exercises");
      searchPath.insert(app_dir + "/share/exercises");
      searchPath.insert(app_dir +  "/../Resources/exercises");
#else
#error Not properly ported.
#endif
    }

  return searchPath;
}


//! Completes the directory for the specified file and file type.
string
Util::complete_directory(string path, Util::SearchPathId type)
{
  string fullPath;
  bool found = false;

  const set<string> &searchPath = get_search_path(type);

  for (set<string>::const_iterator i = searchPath.begin(); !found && i != searchPath.end(); i++)
    {
      fullPath = (*i) + G_DIR_SEPARATOR_S + path;
      found = file_exists(fullPath);
    }

  if (!found)
    {
      fullPath = path;
    }

  return fullPath;
}

bool
Util::running_gnome()
{
  bool ret = false;

#ifdef PLATFORM_OS_UNIX
	gchar *tmp = g_find_program_in_path("gnome-open");

	if (tmp != NULL)
    {
      g_free(tmp);
      tmp = (gchar *)g_getenv("GNOME_DESKTOP_SESSION_ID");

      ret = ((tmp != NULL) && (*tmp != '\0'));
    }
#endif
	return ret;
}

