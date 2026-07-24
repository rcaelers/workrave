// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#import <AppKit/AppKit.h>

#include "ToolkitMacOS.hh"
#include "commonui/MenuDefs.hh"
#include "MacOSDesktopWindow.hh"
#include "ui/macos/MacOSLocker.hh"

#include <QEvent>
#include <QTimer>
#include <QWindow>

// Re-asserts the NSWindow's opaque/background state to match the QWidget's
// Qt::WA_TranslucentBackground attribute. Needed because Qt only decides an
// NSWindow's opacity when the platform window is (re)created, and on macOS a
// window that was ever created translucent can keep rendering with a black
// background even after WA_TranslucentBackground is cleared and the window
// is destroyed/recreated — forcing it here every time is more reliable than
// depending on Qt's own bookkeeping surviving that transition.
static void
configure_native_window(QWindow *qwindow, bool translucent, bool floating)
{
  if (qwindow == nullptr)
    {
      return;
    }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
  auto *view = (__bridge NSView *)(reinterpret_cast<void *>(qwindow->winId()));
  if (view == nil)
    {
      return;
    }
  NSWindow *nswindow = [view window];
  if (nswindow == nil)
    {
      return;
    }

  [nswindow setOpaque:!translucent];
  [nswindow setBackgroundColor:translucent ? [NSColor clearColor] : [NSColor windowBackgroundColor]];

  if (floating)
    {
      [nswindow setCollectionBehavior:NSWindowCollectionBehaviorCanJoinAllSpaces | NSWindowCollectionBehaviorStationary
                                      | NSWindowCollectionBehaviorIgnoresCycle];
      [nswindow setHidesOnDeactivate:NO];
    }
}

int
MacOSMenuStyle::pixelMetric(PixelMetric metric, const QStyleOption *opt, const QWidget *w) const
{
  if (metric == PM_MenuVMargin)
    {
      return QProxyStyle::pixelMetric(metric, opt, w) + 2;
    }
  return QProxyStyle::pixelMetric(metric, opt, w);
}

ToolkitMacOS::ToolkitMacOS(int argc, char **argv)
  : Toolkit(argc, argv)
{
  QApplication::setStyle(new MacOSMenuStyle(QStringLiteral("macos")));
  locker = std::make_shared<MacOSLocker>();
}

void
ToolkitMacOS::init(std::shared_ptr<IApplicationContext> app)
{
  Toolkit::init(app);

  main_window->installEventFilter(this);

  dock_tile = std::make_unique<MacDockTile>(app);
  dock_timer = new QTimer(this);
  connect(dock_timer, &QTimer::timeout, this, [this]() { dock_tile->tick(); });
  dock_timer->start(1000);

  dock_menu = std::make_shared<ToolkitMenu>(app->get_menu_model(),
                                            [](menus::Node::Ptr menu) { return menu->get_id() != MenuId::OPEN; });
  dock_menu->get_menu()->setAsDockMenu();
}

bool
ToolkitMacOS::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == main_window && event->type() == QEvent::WinIdChange)
    {
      bool floating = main_window->windowFlags().testFlag(Qt::FramelessWindowHint) || main_window->windowType() == Qt::Tool;
      configure_native_window(main_window->windowHandle(), main_window->testAttribute(Qt::WA_TranslucentBackground), floating);
    }
  return Toolkit::eventFilter(obj, event);
}

auto
ToolkitMacOS::get_locker() -> std::shared_ptr<Locker>
{
  return locker;
}

auto
ToolkitMacOS::get_desktop_image() -> QPixmap
{
  return MacOSDesktopWindow::get_desktop_image();
}
