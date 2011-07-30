// Locale.hh
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

#ifndef LOCALE_HH
#define LOCALE_HH

#include <map>
#include <string>

#include <glib.h>

using namespace std;

class Locale
{
public:
  struct Language
  {
    std::string language_name;
    std::string country_name;
  };

  typedef std::map<std::string, Language> LanguageMap;
  typedef LanguageMap::iterator LanguageMapIter;
  typedef LanguageMap::const_iterator LanguageMapCIter;


  static bool get_language(const std::string &code, std::string &language);
  static bool get_country(const std::string &code, std::string &language);

  static void get_all_languages_in_current_locale(LanguageMap &list);
  static void get_all_languages_in_native_locale(LanguageMap &list);


  static void set_locale(const std::string &code);
  static std::string get_locale();
  static void lookup(const string &domain, string &str);

  static LanguageMap languages_native_locale;

private:
  void init();

};

#endif // LOCALE_HH
