// Sound.cc --- Sound class
//
// Copyright (C) 2002, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#include "Sound.hh"

#include <X11/Xlib.h>
#include <time.h>

void
Sound::beep(Display *x11, int freq, int millis)
{
  XKeyboardState state;
  XGetKeyboardControl(x11, &state);

  XKeyboardControl values;
  values.bell_pitch = freq;
  // FIXME: why /2 ? The spec claims bell_duration is in millis!!
  values.bell_duration = millis / 2;
  XChangeKeyboardControl(x11, KBBellDuration | KBBellPitch, &values);
  XBell(x11, state.bell_percent);

  values.bell_pitch = state.bell_pitch;
  values.bell_duration = state.bell_duration;
  XChangeKeyboardControl(x11, KBBellDuration | KBBellPitch, &values);

#ifdef HAVE_NANOSLEEP
  struct timespec tv;
  tv.tv_sec = millis / 1000;
  tv.tv_nsec = (millis % 1000) * 1000000;
  nanosleep(&tv, 0);
#else
#  error No nanosleep
#endif
}
