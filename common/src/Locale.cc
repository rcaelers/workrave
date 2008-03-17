// Locale.cc
//
// Copyright (C) 2008 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: Locale.cc 1356 2007-10-22 18:22:13Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <cstdlib>
#include <stdio.h>

#include "Locale.hh"

#include "locale.inc"


int compare_languages (const void *a, const void *b)
{
  return strcmp(((language_t *)a)->code, ((language_t *)b)->code);
}

int compare_countries (const void *a, const void *b)
{
  return strcmp(((country_t*)a)->code, ((country_t*)b)->code);
}

bool
Locale::get_language(const string code, string &language)
{
  language_t key = { code.c_str(), NULL };
  language_t *val;
  
  val = (language_t *) bsearch(&key,
                               languages,
                               sizeof(languages) / sizeof (language_t),
                               sizeof(language_t),
                               compare_languages);
  
  if (val != NULL)
    {
      language = val->lang;
      return true;
    }
  return false;
}

bool
Locale::get_country(const string code, string &country)
{
  country_t key = { code.c_str(), NULL };
  country_t *val;
  
  val = (country_t *) bsearch(&key,
                              countries,
                              sizeof(countries) / sizeof (country_t),
                              sizeof(country_t),
                              compare_countries);
  
  if (val != NULL)
    {
      country = val->country;
      return true;
    }
  return false;
}
