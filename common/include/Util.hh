// Util.hh --- General purpose utility functions
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef UTIL_HH
#define UTIL_HH

#include <string>
#include <list>

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

#ifdef WIN32
  static string get_application_directory();
#endif
  static const list<string> &get_search_path(SearchPathId type);
  static bool file_exists(string path);
  static string complete_directory(string path, SearchPathId type);

private:
  static list<string> search_paths[SEARCH_PATH_SIZEOF];
  static string home_directory;
};

#endif // UTIL_HH
