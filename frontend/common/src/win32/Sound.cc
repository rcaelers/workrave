// Sound.hh --- Sound class
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
// $Id: Sound.cc 1294 2007-08-26 20:48:23Z rcaelers $
//

#include <windows.h>
#include "Sound.hh"

void
Sound::beep(int frequency, int millis)
{
  ::Beep(frequency, millis);
}
