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

#include <gdk/gdkwayland.h>
#include <gtkmm.h>

#include "WaylandLayerShell.hh"

//! gtkmm adapter around the toolkit neutral WaylandLayerShell.
/*!
 *  Its only job is to pull the raw Wayland handles out of GDK and to feed the
 *  compositor's size back into the GtkWindow.
 */
class WaylandWindowManager
{
public:
  WaylandWindowManager() = default;

  bool init();

  //! Creates a layer surface owned by the caller. Returns nullptr if unsupported.
  auto init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus)
    -> std::shared_ptr<WaylandLayerSurface>;

private:
  WaylandLayerShell layer_shell;
};

#endif // WAYLANDWINDOWMANAGER_HH
