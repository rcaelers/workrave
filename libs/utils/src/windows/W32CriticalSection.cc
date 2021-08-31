// W32CriticalSection.cc --- RAII for critical section objects
//
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <windows.h>

#include <list>

#include "utils/W32CriticalSection.hh"

/* A critical section is a fast and light thread synchronization object in Windows. Critical
sections are used by creating a CRITICAL_SECTION object which must be initialized, can be locked
and unlocked multiple times, and then can be deleted.
http://msdn.microsoft.com/en-us/library/windows/desktop/ms682530.aspx

This class, W32CriticalSection, is an RAII wrapper around Microsoft's CRITICAL_SECTION object that
handles the initialization, locking/unlocking and deletion. The locking is thread re-entrant,
unlike Microsoft's own critical_section class which is not.



To use W32CriticalSection:

Add a W32CriticalSection object to your class or somewhere that it's available to your threads.
When creating the object you can specify the spin count, which specifies how many times to try for
the locked object instead of taking a context switch. While the latter sounds expensive, remember
that a context switch isn't needed unless the object is locked. Whether it's worthwhile to set the
spin depends on how many threads are in contention and whether you've done some type of profiling.

You then create a W32CriticalSection::Guard object in any scope of code that you need to lock and
unlock. ***That is important: The W32CriticalSection::Guard object's destructor is called when it
goes out of scope, and it will unlock the W32CriticalSection object that it locked.


Example guard simple:

W32CriticalSection section;   // section's constructor creates and initializes a CRITICAL_SECTION

void SomeFunction()
{
    W32CriticalSection::Guard guard( section );   // guard's constructor locks section here
    shared_thread_variable = 123;   // some awesome guarded code here
}   // the guard's destructor is called on return and it unlocks section here



Advanced:

There are two nested guard classes, W32CriticalSection::AdvancedGuard and W32CriticalSection::Guard.
A single W32CriticalSection::AdvancedGuard object can be used to guard multiple W32CriticalSection
objects and locking and unlocking can be controlled by its methods:

bool TryLock( W32CriticalSection &cs ): Calls TryEnterCriticalSection()
bool TryLockFor( W32CriticalSection &cs, DWORD milliseconds ): Calls TryEnterCriticalSection()
void Lock( W32CriticalSection &cs ): Calls EnterCriticalSection()
void Unlock( W32CriticalSection &cs ): Calls LeaveCriticalSection()

W32CriticalSection::Guard cannot be used on multiple W32CriticalSection objects and has none
of the above methods. Both W32CriticalSection::AdvancedGuard and W32CriticalSection::Guard destruct
similar in that they unlock what that they had locked.

To use W32CriticalSection::AdvancedGuard it is recommended to create and initialize in the scope of
what needs to be locked for the RAII advantage, like W32CriticalSection::Guard (see its example).
You can set an initial lock on construction or you can set one at any time by calling the Lock()
method (you do not need to do both on the same W32CriticalSection object although it's fine to do).

The destructor for W32CriticalSection::AdvancedGuard calls LeaveCriticalSection() for every section
it has locked for the number of times it has not already unlocked. If you lock an object three
times and later unlock it once via Unlock() then when the destructor is called it calls
LeaveCriticalSection() twice for that object. And for multiple objects the unlocking order is
reverse from the locking order. For (a convoluted) example:

W32CriticalSection::AdvancedGuard guard( a );   // calls Lock( a )
guard.Lock( a );
guard.Lock( b );
guard.Lock( a );
guard.Lock( c );
guard.Unlock( a );
The AdvancedGuard object destructor would call LeaveCriticalSection() in this order: c, b, a, a


Example guard advanced:

W32CriticalSection section, section2;

void SomeFunction()
{
    W32CriticalSection::AdvancedGuard guard( section );   // guard's constructor locks section
    shared_thread_variable = 123;   // some awesome guarded code here
    SomeOtherFunction();   // something will happen in some other thread that locks section2 for a bit
    guard.Lock( section2 );   // we're waiting for section2
    mp_update_core();   // some awesome dangerous thing here
}   // when the guard's destructor is called on return it unlocks section and section2 here

*/

W32CriticalSection::AdvancedGuard::AdvancedGuard()
  : W32CriticalSection(500)
{
}

// Lock 'cs' once.
W32CriticalSection::AdvancedGuard::AdvancedGuard(W32CriticalSection &cs)
  : W32CriticalSection(500)
{
  Lock(cs);
}

// Unlock all the locks held that have not yet been unlocked via Unlock().
W32CriticalSection::AdvancedGuard::~AdvancedGuard()
{
  for (std::list<W32CriticalSection *>::reverse_iterator i = list_.rbegin(); i != list_.rend(); ++i)
    {
      LeaveCriticalSection(*i);
    }
}

// Try to lock 'cs' once without blocking for it.
// Returns true if acquired.
bool
W32CriticalSection::AdvancedGuard::TryLock(W32CriticalSection &cs)
{
  if (!TryEnterCriticalSection(&cs))
    return false;

  Guard guard(*this);
  list_.push_back(&cs);
  return true;
}

// Try to lock 'cs' once by blocking for it only for a specific number of 'milliseconds'.
// Returns true if acquired.
bool
W32CriticalSection::AdvancedGuard::TryLockFor(W32CriticalSection &cs, const DWORD milliseconds)
{
  const int spin = (milliseconds ? 4000 : 1);
  const DWORD start = GetTickCount();
  DWORD remaining = milliseconds;

  for (DWORD interval = 1; /**/; interval *= 2)
    {
      for (int i = 0; i < spin; ++i)
        {
          if (TryEnterCriticalSection(&cs))
            {
              Guard guard(*this);
              list_.push_back(&cs);
              return true;
            }

          if (milliseconds != INFINITE)
            {
              DWORD elapsed = GetTickCount() - start;

              if (elapsed >= milliseconds)
                return false;

              remaining = milliseconds - elapsed;
            }
        }

      if ((interval > 128) || (interval > remaining))
        interval = 1;

      Sleep(interval);
    }
}

// Lock 'cs' once by blocking for it.
// Returns after the lock has been acquired.
void
W32CriticalSection::AdvancedGuard::Lock(W32CriticalSection &cs)
{
  EnterCriticalSection(&cs);

  Guard guard(*this);
  list_.push_back(&cs);
}

// Unlock 'cs' once.
// Returns after the lock has been released.
void
W32CriticalSection::AdvancedGuard::Unlock(W32CriticalSection &cs)
{
  Guard guard(*this);

  for (std::list<W32CriticalSection *>::reverse_iterator i = list_.rbegin(); i != list_.rend(); ++i)
    {
      if (*i != &cs)
        continue;

      list_.erase((++i).base());
      LeaveCriticalSection(&cs);
      return;
    }
}
