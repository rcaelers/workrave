// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include "commonui/Text.hh"

#include <cstdio>

#include "commonui/nls.h"

#if defined(PLATFORM_OS_WINDOWS_NATIVE)
#  define snprintf _snprintf
#  define snwprintf _snwprintf
#endif

using namespace std;

//! Converts the specified time to a string
string
Text::time_to_string(time_t time, bool display_units)
{
  char s[128] = "";
  char t[2];

  if (time < 0)
    {
      t[0] = '-';
      t[1] = 0;
      time = -time;
    }
  else
    {
      t[0] = 0;
    }
  int hrs = static_cast<int>(time / 3600);
  int min = (time / 60) % 60;
  int sec = time % 60;

  if (!display_units)
    {
      if (hrs > 0)
        {
          snprintf(s, sizeof(s), "%s%d:%02d:%02d", t, hrs, min, sec);
        }
      else
        {
          snprintf(s, sizeof(s), "%s%d:%02d", t, min, sec);
        }
    }
  else
    {
      if (hrs > 0)
        {
          snprintf(s, sizeof(s), _("%s%d:%02d:%02d hours"), t, hrs, min, sec);
        }
      else if (min > 0)
        {
          snprintf(s, sizeof(s), _("%s%d:%02d minutes"), t, min, sec);
        }
      else
        {
          snprintf(s, sizeof(s), _("%s%d seconds"), t, sec);
        }
    }

  return s;
}
