// Text.cc
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "nls.h"
#include "Text.hh"


//! Time to string
string
Text::time_to_string(time_t time, bool display_units) 
{
  char s[128], t[2];
  
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
  int hrs = time/3600;
  int min = (time / 60) % 60;
  int sec = time % 60;
  
  if (! display_units)
    {
      if (hrs > 0)
        {
          sprintf(s, "%s%ld:%02ld:%02ld", t, hrs, min, sec);
        }
      else
        {
          sprintf(s, "%s%ld:%02ld", t, min, sec);
        }
    }
  else
    {
      if (hrs > 0)
        {
          sprintf(s, _("%s%ld:%02ld:%02ld hours"), t, hrs, min, sec);
        }
      else if (min > 0)
        {
          sprintf(s, _("%s%ld:%02ld minutes"), t, min, sec);
        }
      else
        {
          sprintf(s, _("%s%ld seconds"), t, sec);
        }
    }

  return s;
}

