// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#include "W32LockScreen.hh"

W32LockScreen::LockWorkStationFunc W32LockScreen::lock_func = NULL;
HINSTANCE W32LockScreen::user32_dll = NULL;

W32LockScreen::W32LockScreen()
{
  // Note: this memory is never freed
  user32_dll = LoadLibrary("user32.dll");
  if (user32_dll != NULL)
    {
      lock_func = (LockWorkStationFunc)GetProcAddress(user32_dll, "LockWorkStation");
    }
}

bool
W32LockScreen::lock()
{
  (*lock_func)();
  return true;
}
