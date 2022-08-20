// Copyright (C) 2007 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "MacOSDesktopWindow.hh"

#import <Cocoa/Cocoa.h>

#include <ApplicationServices/ApplicationServices.h>
#import <AppKit/NSRunningApplication.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

extern auto qt_mac_toQImage(CGImageRef image) -> QImage;

auto
MacOSDesktopWindow::get_desktop_image() -> QPixmap
{
  QPixmap pixmap;

  CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll /*OnScreenOnly*/, kCGNullWindowID);
  CFIndex numWindows = CFArrayGetCount(windowList);

  for (int i = 0; i < (int)numWindows; i++)
    {
      const auto *info = (CFDictionaryRef)CFArrayGetValueAtIndex(windowList, i);

      const auto *owner_name = (CFStringRef)CFDictionaryGetValue(info, kCGWindowOwnerName);
      if ((owner_name != nullptr) && CFStringCompare(owner_name, CFSTR("Dock"), 0) == kCFCompareEqualTo)
        {
          const auto *window_name = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
          if ((window_name != nullptr) && (CFStringHasPrefix(window_name, CFSTR("Desktop Picture")) != 0U))
            {
              const auto *index = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowNumber);

              int winId = 0;
              CFNumberGetValue(index, kCFNumberIntType, &winId);

              CGImageRef cgImage = CGWindowListCreateImage(CGRectInfinite,
                                                           kCGWindowListOptionIncludingWindow,
                                                           winId,
                                                           kCGWindowImageNominalResolution);
              // FIXME: don't use internal API
              pixmap = QPixmap::fromImage(qt_mac_toQImage(cgImage));
              break;
            }
        }
    }

  CFRelease(windowList);
  return pixmap;
}
