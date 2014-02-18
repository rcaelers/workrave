// SystemLock.hh -- interface for locking the system
//
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

#ifndef ISYSTEMLOCK_HH
#define ISYSTEMLOCK_HH

class /*interface*/ IScreenLockMethod {
public:
  virtual ~IScreenLockMethod() {};
  virtual bool is_lock_supported() = 0;
  virtual bool lock() = 0;
};

#endif
