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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ui/macos/MacOSSleepMonitor.hh"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

class MacOSSleepMonitor::Pimpl
{
public:
  std::function<void(bool)> callback;
  id sleep_observer{nil};
  id wake_observer{nil};
};

MacOSSleepMonitor::MacOSSleepMonitor(std::function<void(bool)> callback)
  : pimpl(std::make_unique<Pimpl>())
{
  pimpl->callback = std::move(callback);

  const std::function<void(bool)> cb = pimpl->callback;

  NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];

  pimpl->sleep_observer = [center
    addObserverForName:NSWorkspaceWillSleepNotification
                object:[NSWorkspace sharedWorkspace]
                 queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification *) {
              cb(true);
            }];

  pimpl->wake_observer = [center
    addObserverForName:NSWorkspaceDidWakeNotification
                object:[NSWorkspace sharedWorkspace]
                 queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification *) {
              cb(false);
            }];
}

MacOSSleepMonitor::~MacOSSleepMonitor()
{
  NSNotificationCenter *center = [[NSWorkspace sharedWorkspace] notificationCenter];
  [center removeObserver:pimpl->sleep_observer];
  [center removeObserver:pimpl->wake_observer];
}

class MacOSAppNapDisabler::Pimpl
{
public:
  id activity{nil};
};

MacOSAppNapDisabler::MacOSAppNapDisabler()
  : pimpl(std::make_unique<Pimpl>())
{
  // NSActivityUserInitiatedAllowingIdleSystemSleep, held indefinitely: the
  // process is never napped, but the system may still sleep (and decide when).
  pimpl->activity = [[NSProcessInfo processInfo]
    beginActivityWithOptions:NSActivityUserInitiatedAllowingIdleSystemSleep
                      reason:@"Workrave drives its timers from a 1 Hz heartbeat"];
}

MacOSAppNapDisabler::~MacOSAppNapDisabler()
{
  if (pimpl->activity != nil)
    {
      [[NSProcessInfo processInfo] endActivity:pimpl->activity];
      pimpl->activity = nil;
    }
}
