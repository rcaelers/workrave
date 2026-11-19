// Copyright (C) 2026 Wouter van Wijk
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

#ifndef MACOSSLEEPMONITOR_HH
#define MACOSSLEEPMONITOR_HH

#include <functional>
#include <memory>

//! Reports system sleep and wake.
/*!
 *  Subscribes to NSWorkspaceWillSleepNotification / NSWorkspaceDidWakeNotification
 *  and calls the callback with \c true just before the system goes to sleep,
 *  and with \c false when it wakes up.
 */
class MacOSSleepMonitor
{
public:
  explicit MacOSSleepMonitor(std::function<void(bool)> callback);
  ~MacOSSleepMonitor();

  MacOSSleepMonitor(const MacOSSleepMonitor &) = delete;
  MacOSSleepMonitor &operator=(const MacOSSleepMonitor &) = delete;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> pimpl;
};

//! Keeps this process out of App Nap.
/*!
 *  Workrave drives all timers from a 1 Hz heartbeat. Its windows are hidden
 *  most of the time, which makes it a prime App Nap target: the heartbeat is
 *  throttled, the timers run late or stand still, and the core misreads every
 *  gap as a powersave event.
 */
class MacOSAppNapDisabler
{
public:
  MacOSAppNapDisabler();
  ~MacOSAppNapDisabler();

  MacOSAppNapDisabler(const MacOSAppNapDisabler &) = delete;
  MacOSAppNapDisabler &operator=(const MacOSAppNapDisabler &) = delete;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> pimpl;
};

#endif // MACOSSLEEPMONITOR_HH
