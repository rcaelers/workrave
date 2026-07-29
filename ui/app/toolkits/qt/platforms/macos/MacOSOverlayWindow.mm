// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "MacOSOverlayWindow.hh"

#include <unordered_set>

#include <QWindow>

#import <AppKit/NSApplication.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

namespace
{
std::unordered_set<QWindow *> visible_overlays;
NSApplicationActivationPolicy saved_activation_policy = NSApplicationActivationPolicyRegular;
bool activation_policy_changed = false;

NSWindow *
native_window_for(QWindow *window)
{
  auto *view = (__bridge NSView *)(reinterpret_cast<void *>(window->winId()));
  return view.window;
}
} // namespace

void
begin_macos_overlay(QWindow *window)
{
  if (!visible_overlays.insert(window).second)
    {
      return;
    }

  if (visible_overlays.size() == 1)
    {
      saved_activation_policy = NSApp.activationPolicy;
      activation_policy_changed = saved_activation_policy == NSApplicationActivationPolicyRegular
                                  && [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
    }

  // Create the native window while it is still hidden, so every AppKit
  // property is in place before the panel enters another application's Space.
  window->create();
  NSWindow *native_window = native_window_for(window);

  if ([native_window isKindOfClass:NSPanel.class])
    {
      NSPanel *panel = static_cast<NSPanel *>(native_window);
      panel.styleMask |= NSWindowStyleMaskNonactivatingPanel;
      panel.floatingPanel = YES;
      panel.becomesKeyOnlyIfNeeded = YES;
    }

  NSWindowCollectionBehavior collection_behavior =
    NSWindowCollectionBehaviorCanJoinAllSpaces | NSWindowCollectionBehaviorFullScreenAuxiliary |
    NSWindowCollectionBehaviorStationary | NSWindowCollectionBehaviorIgnoresCycle;
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && __MAC_OS_X_VERSION_MAX_ALLOWED >= 130000
  if (@available(macOS 13.0, *))
    {
      collection_behavior |= NSWindowCollectionBehaviorCanJoinAllApplications;
    }
#endif

  native_window.collectionBehavior = collection_behavior;
  native_window.level = NSScreenSaverWindowLevel;
  native_window.ignoresMouseEvents = window->flags().testFlag(Qt::WindowTransparentForInput);
  native_window.hidesOnDeactivate = NO;
}

void
order_macos_overlay_front(QWindow *window)
{
  // Unlike QWindow::raise(), this explicitly leaves the key window, main
  // window, and active application unchanged.
  [native_window_for(window) orderFrontRegardless];
}

void
end_macos_overlay(QWindow *window)
{
  if (visible_overlays.erase(window) == 0 || !visible_overlays.empty())
    {
      return;
    }

  if (activation_policy_changed)
    {
      [NSApp setActivationPolicy:saved_activation_policy];
      activation_policy_changed = false;
    }
}
