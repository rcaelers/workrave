// Uri.cc --- General purpose string utility functions
//
// Copyright (C) 2010, 2011 Rob Caelers
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

#include <cstdlib>
#include <stdio.h>
#include <sstream>

#include <glib.h>

#include "Uri.hh"

using namespace std;

const std::string
Uri::escape(const std::string &in)
{
  string ret;
  char *s = g_uri_escape_string(in.c_str(), NULL, TRUE);
  ret = s;
  g_free(s);
  return ret;
}

const std::string
Uri::unescape(const std::string &in)
{
  string ret;
  char *s = g_uri_unescape_string(in.c_str(), NULL);
  ret = s;
  g_free(s);
  return ret;
}
