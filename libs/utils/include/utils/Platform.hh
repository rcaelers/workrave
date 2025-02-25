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

#if defined(PLATFORM_OS_UNIX) || defined(PLATFORM_OS_WINDOWS)
#  include <string>
#endif

namespace workrave::utils
{
  class Platform
  {
  public:
    static int setenv(const char *name, const char *val, int);
    static int unsetenv(const char *name);
    static bool can_position_windows();
    static bool running_on_wayland();

#if defined(PLATFORM_OS_UNIX)
    static void *get_default_display();
    static std::string get_default_display_name();
    static unsigned long get_default_root_window();
#endif

#if defined(PLATFORM_OS_WINDOWS)
    static bool registry_set_value(const char *path, const char *name, const char *value);
    static bool registry_get_value(const char *path, const char *name, char *out);
    static std::string get_application_name();

  private:
    static std::wstring convert(const char *c);
#endif
  };
} // namespace workrave::utils

#endif // WORKRAVE_UTILS_PLATFORM_HH
