// Copyright (C) 2001 - 2021 Rob Caelers & Raymond Penners
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

#ifndef WINDOWS_LOCKER_HH
#define WINDOWS_LOCKER_HH

#include "ui/Locker.hh"

class WindowsLocker : public Locker
{
public:
  WindowsLocker();
  ~WindowsLocker() override = default;

  bool can_lock() override;
  void prepare_lock() override;
  void lock() override;
  void unlock() override;
};

#endif // WINDOWS_LOCKER_HH
