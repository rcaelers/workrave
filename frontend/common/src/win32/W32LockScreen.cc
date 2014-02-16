/*
 * LockScreen.cc
 *
 *  Created on: 15 lut 2014
 *      Author: mateusz
 */

#include "W32LockScreen.hh"

W32LockScreen::W32LockScreen()
{
  // Note: this memory is never freed
  user32_dll = LoadLibrary("user32.dll");
  if (user32_dll != NULL)
    {
      lock_func = (LockWorkStationFunc)
          GetProcAddress(user32_dll, "LockWorkStation");
    }
}

bool W32LockScreen::lock()
{
  (*lock_func)();
  return true;
}
