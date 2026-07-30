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

#ifndef WAYLANDSCREENPLACEMENT_HH
#define WAYLANDSCREENPLACEMENT_HH

#include <QObject>

class QScreen;
class QWindow;

//! Places a window on a specific screen on Wayland, without staying fullscreen.
/*!
 *  Wayland offers no way for a client to choose its output. The layer shell
 *  protocol solves this, but Mutter does not implement it, so the fallback is
 *  to map the window fullscreen on the wanted output and leave the fullscreen
 *  state again as soon as the compositor confirms it. Staying fullscreen is not
 *  an option: xdg_toplevel.set_fullscreen requires the compositor to hide
 *  everything below a non-opaque surface, which renders a translucent break
 *  window on a black backdrop.
 *
 *  Qt maps Qt::WindowFullScreen to set_fullscreen(screen->output()) for the
 *  window's current QScreen, so setting the screen before showing is what
 *  selects the output.
 *
 *  This is the Qt counterpart of BreakWindow::arm_unfullscreen() in the gtkmm
 *  toolkit. Whether Mutter keeps the window on that output after unfullscreen,
 *  or restores its pre-fullscreen geometry, is unspecified and needs testing.
 */
class WaylandScreenPlacement : public QObject
{
  Q_OBJECT

public:
  //! Arms the placement. Must be called before the window is shown.
  /*!
   *  Does nothing, and leaves the caller to show the window as it sees fit,
   *  unless the window really needs this workaround: Wayland, more than one
   *  screen, and no layer shell.
   *
   *  Returns true when the window was put in the fullscreen state and the
   *  caller should therefore not show it itself.
   */
  static bool arm(QWindow *window, QScreen *screen, bool layer_shell_available);

private:
  explicit WaylandScreenPlacement(QWindow *window);

  void on_window_state_changed(Qt::WindowState state);

  QWindow *window{nullptr};
  bool pending{true};
};

#endif // WAYLANDSCREENPLACEMENT_HH
