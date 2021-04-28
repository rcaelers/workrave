// Copyright (C) 2007 - 2013 Rob Caelers & Raymond Penners
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

#include "ToolkitPlatformMac.hh"

#import <Cocoa/Cocoa.h>

#include <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSRunningApplication.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#if defined(HAVE_QT5)
#  include <QtMacExtras>
#endif

class ToolkitPlatformMac::Pimpl
{
public:
  Pimpl() = default;

  QPixmap get_desktop_image();
  void foreground();
  void restore_foreground();
  void lock();
  void unlock();

private:
  NSRunningApplication *active_app = nil;
  bool active_app_hidden = false;
};

ToolkitPlatformMac::ToolkitPlatformMac()
{
  pimpl = std::make_unique<Pimpl>();
}

ToolkitPlatformMac::~ToolkitPlatformMac()
{
}

QPixmap
ToolkitPlatformMac::get_desktop_image()
{
  return pimpl->get_desktop_image();
}

void
ToolkitPlatformMac::foreground()
{
  pimpl->foreground();
}

void
ToolkitPlatformMac::restore_foreground()
{
  pimpl->restore_foreground();
}

void
ToolkitPlatformMac::lock()
{
  pimpl->lock();
}

void
ToolkitPlatformMac::unlock()
{
  pimpl->unlock();
}

#ifdef HAVE_QT6
extern QImage qt_mac_toQImage(CGImageRef image);
#endif

QPixmap
ToolkitPlatformMac::Pimpl::get_desktop_image()
{
  QPixmap pixmap;

  CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll /*OnScreenOnly*/, kCGNullWindowID);
  CFIndex numWindows = CFArrayGetCount(windowList);

  for (int i = 0; i < (int)numWindows; i++)
    {
      CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);

      CFStringRef owner_name = (CFStringRef)CFDictionaryGetValue(info, kCGWindowOwnerName);
      if (owner_name && CFStringCompare(owner_name, CFSTR("Dock"), 0) == kCFCompareEqualTo)
        {
          CFStringRef window_name = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
          if (window_name && CFStringHasPrefix(window_name, CFSTR("Desktop Picture")))
            {
              CFNumberRef index = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);

              int winId = 0;
              CFNumberGetValue(index, kCFNumberIntType, &winId);

              CGImageRef cgImage =
                CGWindowListCreateImage(CGRectInfinite, kCGWindowListOptionIncludingWindow, winId, kCGWindowImageNominalResolution);
#if defined(HAVE_QT5)
              pixmap = QtMac::fromCGImageRef(cgImage);
#elif defined(HAVE_QT6)
              // FIXME: don't use internal API
              pixmap = QPixmap::fromImage(qt_mac_toQImage(cgImage));
#endif
              break;
            }
        }
    }

  CFRelease(windowList);
  return pixmap;
}

void
ToolkitPlatformMac::Pimpl::foreground()
{
  active_app = [[NSWorkspace sharedWorkspace] frontmostApplication];
  active_app_hidden = [NSApp isHidden];
  [NSApp activateIgnoringOtherApps:YES];
}

void
ToolkitPlatformMac::Pimpl::restore_foreground()
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
ToolkitPlatformMac::Pimpl::lock()
{
  NSApplicationPresentationOptions options =
    (NSApplicationPresentationHideDock | NSApplicationPresentationHideMenuBar | NSApplicationPresentationDisableAppleMenu
     | NSApplicationPresentationDisableProcessSwitching |
     // NSApplicationPresentationDisableForceQuit |
     // NSApplicationPresentationDisableSessionTermination |
     NSApplicationPresentationDisableHideApplication);
  [NSApp setPresentationOptions:options];
}

void
ToolkitPlatformMac::Pimpl::unlock()
{
  [NSApp setPresentationOptions:NSApplicationPresentationDefault];
}
