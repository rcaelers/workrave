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

#include "MacOSPreludeWindow.hh"

#include <QWindow>

#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

void
show_macos_prelude_without_activation(QWindow *window)
{
  // Create the native window while it is still hidden, so it can be made
  // click-through before AppKit places it on screen.
  window->create();

  auto *view             = (__bridge NSView *)(reinterpret_cast<void *>(window->winId()));
  NSWindow *native_window = view.window;
  native_window.ignoresMouseEvents = YES;
  native_window.hidesOnDeactivate  = NO;

  window->show();

  // Unlike QWindow::raise(), this explicitly leaves the key window, main
  // window, and active application unchanged.
  [native_window orderFrontRegardless];
}
