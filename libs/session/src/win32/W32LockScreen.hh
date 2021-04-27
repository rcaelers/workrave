// LockScreen.hh - locking the screen on Windows
// Copyright (C) 2014 Mateusz Jończyk <mat.jonczyk@o2.pl>
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
//

#ifndef LOCKSCREEN_HH_
#define LOCKSCREEN_HH_

#include "session/IScreenLockMethod.hh"

#include <windows.h>
#include <stdlib.h>

class W32LockScreen : public IScreenLockMethod
{
public:
  W32LockScreen();
  virtual ~W32LockScreen(){};
  virtual bool is_lock_supported()
  {
    return lock_func != NULL;
  };
  virtual bool lock();

private:
  typedef HRESULT(FAR PASCAL *LockWorkStationFunc)(void);
  static LockWorkStationFunc lock_func;
  static HINSTANCE user32_dll;
};

#endif /* LOCKSCREEN_HH_ */
