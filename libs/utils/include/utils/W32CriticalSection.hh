// Copyright (C) 2012 Ray Satiro <raysatiro@yahoo.com>
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

#ifndef W32CRITICALSECTION_HH
#define W32CRITICALSECTION_HH

#include <windows.h>

#include <list>

class W32CriticalSection : public CRITICAL_SECTION
{
public:
  W32CriticalSection()
  {
    InitializeCriticalSection(this);
  }
  W32CriticalSection(DWORD dwSpinCount)
  {
    InitializeCriticalSectionAndSpinCount(this, dwSpinCount);
  }
  ~W32CriticalSection()
  {
    DeleteCriticalSection(this);
  }

  class Guard
  {
  public:
    explicit Guard(W32CriticalSection &cs)
      : critsec_(cs)
    {
      EnterCriticalSection(&critsec_);
    }
    ~Guard()
    {
      LeaveCriticalSection(&critsec_);
    }

  private:
    W32CriticalSection &critsec_;

    Guard(const Guard &);
    Guard &operator=(const Guard &);
  };

  class AdvancedGuard;

private:
  W32CriticalSection(const W32CriticalSection &);
  W32CriticalSection &operator=(const W32CriticalSection &);
};

class W32CriticalSection::AdvancedGuard : public W32CriticalSection
{
public:
  AdvancedGuard();
  explicit AdvancedGuard(W32CriticalSection &cs);
  ~AdvancedGuard();

  bool TryLock(W32CriticalSection &cs);
  bool TryLockFor(W32CriticalSection &cs, const DWORD milliseconds);
  void Lock(W32CriticalSection &cs);
  void Unlock(W32CriticalSection &cs);

private:
  std::list<W32CriticalSection *> list_;

  AdvancedGuard(const AdvancedGuard &);
  AdvancedGuard &operator=(const AdvancedGuard &);
};

#endif // W32CRITICALSECTION_HH
