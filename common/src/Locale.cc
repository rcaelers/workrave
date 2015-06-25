// Locale.cc
//
// Copyright (C) 2008, 2010 Rob Caelers <robc@krandor.nl>
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
#include "nls.h"

#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <string.h>

#include "Locale.hh"
#include "StringUtil.hh"

#include "locale.inc"

extern "C" int _nl_msg_cat_cntr;

Locale::LanguageMap Locale::languages_native_locale;

int compare_languages (const void *a, const void *b)
{
  return strcmp(((language_t *)a)->code, ((language_t *)b)->code);
}

int compare_countries (const void *a, const void *b)
{
  return strcmp(((country_t*)a)->code, ((country_t*)b)->code);
}

bool
Locale::get_language(const string &code, string &language)
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
Locale::get_country(const string &code, string &country)
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

void
Locale::set_locale(const std::string &code)
{
  if (code != "")
    {
      g_setenv("LANGUAGE", code.c_str(), 1);
      g_setenv("LANG", code.c_str(), 1);
    }
  else
    {
      g_unsetenv("LANGUAGE");
      g_unsetenv("LANG");
    }

#ifndef PLATFORM_OS_WIN32_NATIVE
    ++_nl_msg_cat_cntr;
#endif
}

std::string
Locale::get_locale()
{
  string ret;
  const char *lang_env = g_getenv("LANGUAGE");

  if (lang_env == NULL)
    {
      lang_env = g_getenv("LANG");
    }

  if (lang_env != NULL)
    {
      ret = lang_env;
    }

  return ret;
}

void
Locale::lookup(const string &domain, string &str)
{
  string ret;

  if (str != "")
    {
      ret = dgettext(domain.c_str(), str.c_str());
      str = ret;
    }
}

void
Locale::init()
{
}

void
Locale::get_all_languages_in_current_locale(LanguageMap &languages)
{
  std::vector<std::string> all_linguas;

#ifdef HAVE_LANGUAGE_SELECTION
  StringUtil::split(string(ALL_LINGUAS), ' ', all_linguas);
  all_linguas.push_back("en");

  for (vector<std::string>::iterator i = all_linguas.begin(); i != all_linguas.end(); i++)
    {
      string code = *i;
      string lang_code;
      string country_code;

      Language &language_entry = languages[code];

      lang_code = code.substr(0,2);
      if (code.length() >= 5)
        {
          country_code = code.substr(3,2);
        }

      Locale::get_language(lang_code, language_entry.language_name);
      Locale::get_country(country_code, language_entry.country_name);

      Locale::lookup("iso_639", language_entry.language_name);
      Locale::lookup("iso_3166", language_entry.country_name);
    }
#endif  
}


void
Locale::get_all_languages_in_native_locale(LanguageMap &list)
{
#ifdef HAVE_LANGUAGE_SELECTION
  static bool init_done = false;

  if (init_done)
    {
      list = languages_native_locale;
      return;
    }

  std::vector<std::string> all_linguas;

  StringUtil::split(string(ALL_LINGUAS), ' ', all_linguas);
  all_linguas.push_back("en");

  string lang_save = Locale::get_locale();

  for (vector<std::string>::iterator i = all_linguas.begin(); i != all_linguas.end(); i++)
    {
      string code = *i;
      string lang_code;
      string country_code;

      Locale::set_locale(code);

      Language &language_entry = languages_native_locale[code];

      lang_code = code.substr(0,2);
      if (code.length() >= 5)
        {
          country_code = code.substr(3,2);
        }

      Locale::get_language(lang_code, language_entry.language_name);
      Locale::get_country(country_code, language_entry.country_name);

      Locale::lookup("iso_639", language_entry.language_name);
      Locale::lookup("iso_3166", language_entry.country_name);
    }

  init_done = true;
  Locale::set_locale(lang_save);
  list = languages_native_locale;
#endif
}

#ifdef PLATFORM_OS_WIN32
#include <windows.h>
#endif

#ifdef PLATFORM_OS_UNIX
#include <langinfo.h>
#endif

int
Locale::get_week_start()
{
  int week_start = 0;
  
#ifdef PLATFORM_OS_WIN32
  WCHAR wsDay[4];
	if (
#  if defined(_WIN32_WINNT_VISTA) && WINVER >= _WIN32_WINNT_VISTA && defined(LOCALE_NAME_USER_DEFAULT)
      GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, wsDay, 4)
#  else
      GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, wsDay, 4)
#  endif
      )
    {
      char *ws = g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(wsDay), -1, NULL, NULL, NULL);
      if (ws != NULL)
        {
          week_start = (ws[0] - '0' + 1) % 7;
          g_free(ws);
        }
    }
#endif

#ifdef PLATFORM_OS_UNIX
  union { unsigned int word; char *string; } langinfo;
  gint week_1stday = 0;
  gint first_weekday = 1;
  guint week_origin;
  
  langinfo.string = nl_langinfo(_NL_TIME_FIRST_WEEKDAY);
  first_weekday = langinfo.string[0];
  langinfo.string = nl_langinfo(_NL_TIME_WEEK_1STDAY);
  week_origin = langinfo.word;
  if (week_origin == 19971130) /* Sunday */
    week_1stday = 0;
  else if (week_origin == 19971201) /* Monday */
    week_1stday = 1;
  else
    g_warning ("Unknown value of _NL_TIME_WEEK_1STDAY.\n");

  week_start = (week_1stday + first_weekday - 1) % 7;
#endif

  return week_start;
}


