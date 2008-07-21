// Sound.cc --- Sound class
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
// $Id$
//

#include "Sound.hh"
#include "Thread.hh"

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
  values.bell_duration = millis/2;
  XChangeKeyboardControl(x11, KBBellDuration|KBBellPitch, &values);
  XBell(x11, state.bell_percent);

  values.bell_pitch = state.bell_pitch;
  values.bell_duration = state.bell_duration;
  XChangeKeyboardControl(x11, KBBellDuration|KBBellPitch, &values);

  Thread::sleep(millis, 0);
}
