// Util.cc --- General purpose utility functions
//
// Copyright (C) 2001, 2002, 2003, 2006 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <stdio.h>
#include <sstream>

#include <unistd.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef WIN32
#include <windows.h>
// HACK: #include <shlobj.h>, need -fvtable-thunks.
// Perhaps we should enable this, but let's hack it for now...
//#include <shlobj.h>
extern "C" {
#define SHGetPathFromIDList SHGetPathFromIDListA
HRESULT WINAPI SHGetSpecialFolderLocation(HWND,int,void**);
BOOL WINAPI SHGetPathFromIDList(void *,LPSTR);
VOID WINAPI CoTaskMemFree(PVOID);
#define PathCanonicalize PathCanonicalizeA
BOOL WINAPI PathCanonicalize(LPSTR,LPCSTR);
#define CSIDL_APPDATA   26
}
// (end of hack)
#endif

#include "Util.hh"

list<string> Util::search_paths[Util::SEARCH_PATH_SIZEOF];
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
  
#if defined(HAVE_X)  
      const char *home = getenv("WORKRAVE_HOME");
  
      if (home == NULL)
        {
          home = getenv("HOME");
        }
      
      if (home != NULL)
        {
          ret = home;
          ret += "/.workrave/";
          
          mkdir(ret.c_str(), 0777);
        }
#elif defined(WIN32)
      void *pidl; 
      char buf[MAX_PATH];
      HRESULT hr = SHGetSpecialFolderLocation(HWND_DESKTOP,
                                              CSIDL_APPDATA, &pidl);
      if (SUCCEEDED(hr))
        {
          SHGetPathFromIDList(pidl, buf);
          CoTaskMemFree(pidl);
          
          strcat (buf, "\\Workrave");
          bool dirok = false;
          dirok = CreateDirectory(buf, NULL);
          if (! dirok)
            {
              if (GetLastError() == ERROR_ALREADY_EXISTS)
                {
                  dirok = true;
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
#ifdef WIN32
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

#ifdef WIN32
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


#ifdef WIN32
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
#endif


//! Returns the searchpath for the specified file type.
const list<string> &
Util::get_search_path(SearchPathId type)
{
  if (search_paths[type].size() > 0)
    return search_paths[type];
  
  list<string> &searchPath = search_paths[type];

  string home_dir = get_home_directory();
#if defined(WIN32)
  string app_dir = get_application_directory();
#endif
  
  searchPath.push_back("./");
  
  if (type == SEARCH_PATH_IMAGES)
    {
#if defined(HAVE_X)
      if (home_dir != "./")
        {
          searchPath.push_back(home_dir + "/");
          searchPath.push_back(home_dir + "/images");
        }
      searchPath.push_back(string(WORKRAVE_DATADIR) + "/images");
      searchPath.push_back("/usr/local/share/workrave/images");
      searchPath.push_back("/usr/share/workrave/images");
#elif defined(WIN32)
      searchPath.push_back(string(app_dir) + "\\share\\images");
#endif    
    }
  else if (type == SEARCH_PATH_CONFIG)
    {
#if defined(HAVE_X)
      if (home_dir != "./")
        {
          searchPath.push_back(home_dir + "/");
          searchPath.push_back(home_dir + "/etc");
        }
      searchPath.push_back(string(WORKRAVE_DATADIR) + "/etc");
      searchPath.push_back("/usr/local/share/workrave/etc");
      searchPath.push_back("/usr/share/workrave/etc");
#elif defined(WIN32)
      searchPath.push_back(home_dir + "/");
      searchPath.push_back(string(app_dir) + "\\etc");
#endif    
    }
  else if (type == SEARCH_PATH_EXERCISES)
    {
#if defined(HAVE_X)
      searchPath.push_back(string(WORKRAVE_DATADIR) + "/exercises");
#elif defined(WIN32)
      searchPath.push_back(string(app_dir) + "\\share\\exercises");
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

  const list<string> &searchPath = get_search_path(type);

  for (list<string>::const_iterator i = searchPath.begin(); !found && i != searchPath.end(); i++)
    {
      fullPath = (*i) + "/" + path;
      found = file_exists(fullPath);
    }

  if (!found)
    {
      fullPath = path;
    }

  return fullPath;
}
