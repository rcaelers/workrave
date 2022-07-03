// Copyright (C) 2008, 2010, 2013 Rob Caelers <robc@krandor.nl>
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

#include "commonui/Locale.hh"

#include "debug.hh"
#include "commonui/nls.h"

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cstring>
#include <boost/algorithm/string.hpp>

#include "utils/Platform.hh"

#include "locale.inc"

using namespace workrave::utils;

extern "C" int _nl_msg_cat_cntr;

Locale::LanguageMap Locale::languages_native_locale;

int
compare_languages(const void *a, const void *b)
{
  return strcmp(static_cast<const language_t *>(a)->code, static_cast<const language_t *>(b)->code);
}

int
compare_countries(const void *a, const void *b)
{
  return strcmp(static_cast<const country_t *>(a)->code, static_cast<const country_t *>(b)->code);
}

bool
Locale::get_language(const std::string &code, std::string &language)
{
  language_t key = {code.c_str(), nullptr};
  language_t *val = nullptr;

  val = reinterpret_cast<language_t *>(
    bsearch(&key, languages, sizeof(languages) / sizeof(language_t), sizeof(language_t), compare_languages));

  if (val != nullptr)
    {
      language = val->lang;
      return true;
    }
  return false;
}

bool
Locale::get_country(const std::string &code, std::string &country)
{
  country_t key = {code.c_str(), nullptr};
  country_t *val;

  val = reinterpret_cast<country_t *>(
    bsearch(&key, countries, sizeof(countries) / sizeof(country_t), sizeof(country_t), compare_countries));

  if (val != nullptr)
    {
      country = val->country;
      return true;
    }
  return false;
}

void
Locale::set_locale(const std::string &code)
{
  if (!code.empty())
    {
      Platform::setenv("LANGUAGE", code.c_str(), 1);
      Platform::setenv("LANG", code.c_str(), 1);
    }
  else
    {
      Platform::unsetenv("LANGUAGE");
      Platform::unsetenv("LANG");
    }

#if !defined(PLATFORM_OS_WINDOWS_NATIVE)
  ++_nl_msg_cat_cntr;
#endif
}

std::string
Locale::get_locale()
{
  std::string ret;
  const char *lang_env = getenv("LANGUAGE");

  if (lang_env == nullptr)
    {
      lang_env = getenv("LANG");
    }

  if (lang_env != nullptr)
    {
      ret = lang_env;
    }

  return ret;
}

void
Locale::lookup(const std::string &domain, std::string &str)
{
  std::string ret;

  if (!str.empty())
    {
#if defined(HAVE_LIBINTL)
      ret = dgettext(domain.c_str(), str.c_str());
      str = ret;
#elif defined(HAVE_QT)
      ret = QObject::tr(str.c_str()).toStdString();
      str = ret;
#endif
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

  boost::split(all_linguas, ALL_LINGUAS, boost::is_any_of(" "));
  (void)languages;
  all_linguas.emplace_back("en");

  for (auto code: all_linguas)
    {
      std::string lang_code;
      std::string country_code;

      Language &language_entry = languages[code];

      lang_code = code.substr(0, 2);
      if (code.length() >= 5)
        {
          country_code = code.substr(3, 2);
        }

      Locale::get_language(lang_code, language_entry.language_name);
      Locale::get_country(country_code, language_entry.country_name);

      Locale::lookup("iso_639", language_entry.language_name);
      Locale::lookup("iso_3166", language_entry.country_name);
    }
}

void
Locale::get_all_languages_in_native_locale(LanguageMap &list)
{
  (void)list;

#if defined(HAVE_LANGUAGE_SELECTION)
  static bool init_done = false;

  if (init_done)
    {
      list = languages_native_locale;
      return;
    }

  std::vector<std::string> all_linguas;

  boost::split(all_linguas, ALL_LINGUAS, boost::is_any_of(" "));
  all_linguas.emplace_back("en");

  std::string lang_save = Locale::get_locale();

  for (auto code: all_linguas)
    {
      std::string lang_code;
      std::string country_code;

      Locale::set_locale(code);

      Language &language_entry = languages_native_locale[code];

      lang_code = code.substr(0, 2);
      if (code.length() >= 5)
        {
          country_code = code.substr(3, 2);
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

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#endif

#if defined(PLATFORM_OS_UNIX)
#  include <langinfo.h>
#  include <glib.h>
#endif

#if defined(PLATFORM_OS_MACOS)
#  import <Foundation/NSCalendar.h>
#endif
int
Locale::get_week_start()
{
  int week_start = 0;

#if defined(PLATFORM_OS_WINDOWS)
  WCHAR wsDay[4];
  if (
#  if defined(_WIN32_WINNT_VISTA) && WINVER >= _WIN32_WINNT_VISTA && defined(LOCALE_NAME_USER_DEFAULT)
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, wsDay, 4)
#  else
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, wsDay, 4)
#  endif
  )
    {
      int required_size = WideCharToMultiByte(CP_UTF8, 0, wsDay, -1, nullptr, 0, nullptr, nullptr);
      if (required_size > 0)
        {
          std::vector<char> buffer(required_size);
          WideCharToMultiByte(CP_UTF8, 0, wsDay, -1, &buffer[0], required_size, nullptr, nullptr);
          week_start = (buffer[0] - '0' + 1) % 7;
        }
    }

#elif defined(PLATFORM_OS_MACOS)
  week_start = [[NSCalendar currentCalendar] firstWeekday];

#elif defined(PLATFORM_OS_UNIX)
  union
  {
    unsigned int word;
    char *string;
  } langinfo;
  int week_1stday = 0;
  int first_weekday = 1;
  unsigned int week_origin;

  langinfo.string = nl_langinfo(_NL_TIME_FIRST_WEEKDAY);
  first_weekday = langinfo.string[0];
  langinfo.string = nl_langinfo(_NL_TIME_WEEK_1STDAY);
  week_origin = langinfo.word;
  if (week_origin == 19971130) /* Sunday */
    week_1stday = 0;
  else if (week_origin == 19971201) /* Monday */
    week_1stday = 1;
  else
    g_warning("Unknown value of _NL_TIME_WEEK_1STDAY.\n");

  week_start = (week_1stday + first_weekday - 1) % 7;
#endif

  return week_start;
}
