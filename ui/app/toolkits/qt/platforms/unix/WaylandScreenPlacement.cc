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
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "WaylandScreenPlacement.hh"

#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include "debug.hh"
#include "utils/Platform.hh"

namespace
{
  //! Delay before dropping the fullscreen state again.
  /*!
   *  Leaving it in the same commit lets the compositor coalesce both requests,
   *  in which case the window never moves to the wanted output at all.
   */
  constexpr int unfullscreen_delay_ms = 100;
} // namespace

bool
WaylandScreenPlacement::arm(QWindow *window, QScreen *screen, bool layer_shell_available)
{
  TRACE_ENTRY();
  if (window == nullptr || screen == nullptr)
    {
      return false;
    }

  if (!workrave::utils::Platform::running_on_wayland())
    {
      return false;
    }

  // The layer shell already anchors the surface to the right output.
  if (layer_shell_available)
    {
      return false;
    }

  if (QGuiApplication::screens().size() <= 1)
    {
      return false;
    }

  window->setScreen(screen);

  // Owned by the window: the connection, and this helper, die with it.
  new WaylandScreenPlacement(window);

  window->showFullScreen();
  return true;
}

WaylandScreenPlacement::WaylandScreenPlacement(QWindow *window)
  : QObject(window)
  , window(window)
{
  connect(window, &QWindow::windowStateChanged, this, &WaylandScreenPlacement::on_window_state_changed);
}

void
WaylandScreenPlacement::on_window_state_changed(Qt::WindowState state)
{
  if (!pending || state != Qt::WindowFullScreen)
    {
      return;
    }

  TRACE_ENTRY();
  pending = false;

  // Leave the fullscreen state only after the compositor has presented a frame.
  QTimer::singleShot(unfullscreen_delay_ms, this, [this]() {
    if (window != nullptr)
      {
        window->showNormal();
      }
    deleteLater();
  });
}
