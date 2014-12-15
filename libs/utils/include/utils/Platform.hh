// Platform.hh --- Base exception
//
// Copyright (C) 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_PLATFORM_HH
#define WORKRAVE_UTILS_PLATFORM_HH

#include <string>

namespace workrave
{
  namespace utils
  {
    class Platform
    {
    public:
#ifdef PLATFORM_OS_UNIX
      static void *get_default_display();
      static unsigned long get_default_root_window();
#endif

#ifdef PLATFORM_OS_WIN32
      static std::string get_application_directory();
      static std::string get_application_name();
      static bool registry_set_value(const char *path, const char *name, const char *value);
      static bool registry_get_value(const char *path, const char *name, char *out);
    private:
      static std::wstring convert(const char* c);
#endif
    public:
      static int setenv(const char* name, const char* val, int);
      static int unsetenv(const char* name);
    };
  }
}

#endif // WORKRAVE_UTILS_PLATFORM_HH
