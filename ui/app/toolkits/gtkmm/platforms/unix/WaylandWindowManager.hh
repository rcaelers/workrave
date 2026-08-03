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

#ifndef WAYLANDWINDOWMANAGER_HH
#define WAYLANDWINDOWMANAGER_HH

#include <memory>

#include <gdk/gdk.h>
#if GDK_MAJOR_VERSION >= 4
#  include <gdk/wayland/gdkwayland.h>
#else
#  include <gdk/gdkwayland.h>
#endif
#include <gtkmm.h>

#include "WaylandLayerShell.hh"

#if GTK_CHECK_VERSION(4, 0, 0) && defined(HAVE_GTK4_LAYER_SHELL)
#  include <gtk4-layer-shell/gtk4-layer-shell.h>
#endif

//! gtkmm adapter around the layer shell.
/*!
 *  On GTK3, pulls the raw Wayland handles out of GDK and drives the toolkit
 *  neutral WaylandLayerShell/zwlr_layer_shell_v1 binding directly, feeding the
 *  compositor's negotiated size back into the GtkWindow.
 *
 *  On GTK4, GDK no longer exposes the hook (gdk_wayland_window_set_use_custom_surface)
 *  needed to hand a raw surface to the layer shell, so this instead relies on
 *  gtk4-layer-shell, which works around the same problem at the Wayland
 *  protocol level and drives the GtkWindow directly.
 */
class WaylandWindowManager
{
public:
  WaylandWindowManager() = default;

  bool init();

#if GTK_CHECK_VERSION(4, 0, 0) && defined(HAVE_GTK4_LAYER_SHELL)
  //! Turns gtk_window into a layer surface anchored to every edge of monitor.
  //! Must be called before gtk_window is realized, since gtk4-layer-shell
  //! requires that. Unlike init_surface() below, the layer surface's lifetime
  //! is tied to the GtkWindow itself: there is no separate handle to keep
  //! around, and GTK's normal size negotiation resizes the window once
  //! anchored, so no configure callback is needed either.
  void setup_surface(Gtk::Window &gtk_window, const Glib::RefPtr<Gdk::Monitor> &monitor, bool keyboard_focus);
#else
  //! Creates a layer surface owned by the caller. Returns nullptr if unsupported.
  auto init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus)
    -> std::shared_ptr<WaylandLayerSurface>;

private:
  WaylandLayerShell layer_shell;
#endif
};

#endif // WAYLANDWINDOWMANAGER_HH
