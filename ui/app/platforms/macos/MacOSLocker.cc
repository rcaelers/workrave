// Copyright (C) 2007 - 2021 Rob Caelers & Raymond Penners
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

#include "ui/macos/MacOSLocker.hh"

#import <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSRunningApplication.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include <cstdio>
#include "debug.hh"
#include "utils/Platform.hh"

using namespace workrave::utils;

class MacOSLocker::Pimpl
{
public:
  Pimpl() = default;

  void foreground();
  void restore_foreground();
  bool can_lock();
  void lock();
  void unlock();

private:
  NSRunningApplication *active_app = nil;
  bool active_app_hidden = false;
};

MacOSLocker::MacOSLocker()
{
  pimpl = std::make_unique<Pimpl>();
}

MacOSLocker::~MacOSLocker()
{
}

bool
MacOSLocker::can_lock()
{
  return true;
}

void
MacOSLocker::prepare_lock()
{
}

void
MacOSLocker::lock()
{
  pimpl->foreground();
  pimpl->lock();
}

void
MacOSLocker::unlock()
{
  pimpl->unlock();
  pimpl->restore_foreground();
}

void
MacOSLocker::Pimpl::foreground()
{
  active_app = [[NSWorkspace sharedWorkspace] frontmostApplication];
  active_app_hidden = [NSApp isHidden];
  [NSApp activateIgnoringOtherApps:YES];
}

void
MacOSLocker::Pimpl::restore_foreground()
{
  if (active_app != nil)
    {
      [active_app activateWithOptions:NSApplicationActivateIgnoringOtherApps];
      if (active_app_hidden)
        {
          [NSApp hide:active_app];
        }
      active_app = nil;
    }
}

void
MacOSLocker::Pimpl::lock()
{
  NSApplicationPresentationOptions options = (NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar
                                              | NSApplicationPresentationDisableAppleMenu
                                              | NSApplicationPresentationDisableProcessSwitching |
                                              // NSApplicationPresentationDisableForceQuit |
                                              // NSApplicationPresentationDisableSessionTermination |
                                              NSApplicationPresentationDisableHideApplication);
  [NSApp setPresentationOptions:options];
}

void
MacOSLocker::Pimpl::unlock()
{
  [NSApp setPresentationOptions:NSApplicationPresentationDefault];
}
