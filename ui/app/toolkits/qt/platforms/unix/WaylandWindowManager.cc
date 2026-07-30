// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#include "WaylandWindowManager.hh"

#include <memory>

#include <QScreen>

#include <qguiapplication_platform.h>
#include <QtGui/qpa/qplatformwindow_p.h>

#include <QtWaylandClient/private/qwaylandscreen_p.h>

#include "debug.hh"

bool
WaylandWindowManager::init()
{
  TRACE_ENTRY();
  auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
  if (app == nullptr)
    {
      TRACE_MSG("not running on wayland");
      return false;
    }

  return layer_shell.init(app->display());
}

auto
WaylandWindowManager::init_surface(QWidget *window, QScreen *screen, bool keyboard_focus)
  -> std::shared_ptr<WaylandLayerSurface>
{
  TRACE_ENTRY();
  if (window == nullptr)
    {
      return {};
    }
  return create_surface(window->windowHandle(), screen, keyboard_focus);
}

auto
WaylandWindowManager::init_surface(QWindow *window, QScreen *screen, bool keyboard_focus)
  -> std::shared_ptr<WaylandLayerSurface>
{
  TRACE_ENTRY();
  // A QWindow (e.g. QQuickView) exposes the Wayland window directly.
  return create_surface(window, screen, keyboard_focus);
}

auto
WaylandWindowManager::create_surface(QWindow *window, QScreen *screen, bool keyboard_focus)
  -> std::shared_ptr<WaylandLayerSurface>
{
  if (!layer_shell.is_supported() || window == nullptr || screen == nullptr)
    {
      return {};
    }

  auto *wayland_window = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
  auto *wayland_screen = screen->nativeInterface<QtWaylandClient::QWaylandScreen>();
  if (wayland_window == nullptr || wayland_screen == nullptr)
    {
      TRACE_MSG("not running on wayland");
      return {};
    }

  return layer_shell.create_surface(wayland_window->surface(),
                                    wayland_screen->output(),
                                    keyboard_focus,
                                    [window](int width, int height) { window->resize(width, height); });
}
