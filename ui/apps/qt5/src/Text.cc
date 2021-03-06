// Text.cc
//
// Copyright (C) 2002, 2003, 2007, 2008, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include "Text.hh"

#include <stdio.h>

#ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
#  ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#include "qformat.hh"

//! Converts the specified time to a string
QString
Text::time_to_string(time_t time, bool display_units)
{
  QString ret;

  if (time < 0)
    {
      ret += '-';
      time = -time;
    }

  int hrs = static_cast<int>(time / 3600);
  int min = (time / 60) % 60;
  int sec = time % 60;

  if (!display_units)
    {
      if (hrs > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d:%02d")) % "" % hrs % min % sec);
        }
      else
        {
          ret += qstr(qformat(tr("%s%d:%02d")) % "" % min % sec);
        }
    }
  else
    {
      if (hrs > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d:%02d hours")) % "" % hrs % min % sec);
        }
      else if (min > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d minutes")) % "" % min % sec);
        }
      else
        {
          ret += qstr(qformat(tr("%s%d seconds")) % "" % sec);
        }
    }

  return ret;
}
