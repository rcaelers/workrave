// Sound.cc --- Sound class
//
// Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// Time-stamp: <2002-10-06 18:31:05 pennersr>
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
Sound::beep(int freq, int millis)
{
#if 0  
  // FIXME: what about a KDE port?
  extern Display *gdk_display;

  XLockDisplay(gdk_display);
  
  XKeyboardState state;
  XGetKeyboardControl(gdk_display, &state);
  
  XKeyboardControl values;
  values.bell_pitch = freq;
  // FIXME: why /2 ? The spec claims bell_duration is in millis!!
  values.bell_duration = millis/2;
  XChangeKeyboardControl(gdk_display, KBBellDuration|KBBellPitch, &values);
  XBell(gdk_display, state.bell_percent);

  values.bell_pitch = state.bell_pitch;
  values.bell_duration = state.bell_duration;
  XChangeKeyboardControl(gdk_display, KBBellDuration|KBBellPitch, &values);

  XUnlockDisplay(gdk_display);
#endif  
  Thread::sleep(millis, 0);
}
