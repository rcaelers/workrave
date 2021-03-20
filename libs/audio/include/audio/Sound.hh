// Sound.hh --- Sound class
//
// Copyright (C) 2002, 2003, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef SOUND_HH
#define SOUND_HH

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef PLATFORM_OS_UNIX
#  include <X11/Xlib.h>
#endif

class Sound
{
public:
#ifdef PLATFORM_OS_UNIX
  static void beep(Display *x11, int frequency, int millis);
#else
  static void beep(int frequency, int millis);
#endif
};

#endif // SOUND_HH
