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

#include <gdk/gdkwayland.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gtkmm.h>
#include <wayland-client-core.h>

#include "wlr-layer-shell-unstable-v1-client.h"

class LayerSurface
{
public:
  LayerSurface(struct zwlr_layer_shell_v1 *layer_shell, Gtk::Widget &window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus);
  ~LayerSurface() = default;

private:
  static void layer_surface_configure(void *data,
                                      struct zwlr_layer_surface_v1 *surface,
                                      uint32_t serial,
                                      uint32_t width,
                                      uint32_t height);

  static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface);

private:
  struct zwlr_layer_shell_v1 *layer_shell{};
  struct zwlr_layer_surface_v1 *layer_surface{};
  struct wl_display *display{};
  GtkWidget *gtk_window{};
  bool keyboard_focus{true};

  static constexpr const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = LayerSurface::layer_surface_configure,
    .closed = LayerSurface::layer_surface_closed,
  };
};

class WaylandWindowManager
{
public:
  WaylandWindowManager() = default;
  ~WaylandWindowManager();

  bool init();

  void init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor>, bool keyboard_focus);
  void clear_surfaces();

public:
  static void registry_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
  static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t id);

private:
  struct wl_registry *wl_registry{};
  struct zwlr_layer_shell_v1 *layer_shell{};
  std::list<std::shared_ptr<LayerSurface>> surfaces{};
};

#endif // WAYLANDWINDOWMANAGER_HH
