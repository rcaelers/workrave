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
#include <gdk/gdkwayland.h>

#include <spdlog/spdlog.h>

#include "debug.hh"

static const struct wl_registry_listener registry_listener = {
  .global = WaylandWindowManager::registry_global,
  .global_remove = WaylandWindowManager::registry_global_remove,
};

WaylandWindowManager::~WaylandWindowManager()
{
  TRACE_ENTRY();
  // Layer surfaces are owned by the windows and stay valid after the shell is
  // destroyed, as required by zwlr_layer_shell_v1.destroy.
  if (layer_shell != nullptr)
    {
      zwlr_layer_shell_v1_destroy(layer_shell);
      layer_shell = nullptr;
    }
  if (wl_registry != nullptr)
    {
      wl_registry_destroy(wl_registry);
      wl_registry = nullptr;
    }
}

bool
WaylandWindowManager::init()
{
  TRACE_ENTRY();
  auto *gdk_display = gdk_display_get_default();
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition,cppcoreguidelines-pro-type-cstyle-cast)
  if (!GDK_IS_WAYLAND_DISPLAY(gdk_display))
    {
      TRACE_MSG("not running on wayland");
      return false;
    }

  auto *wl_display = gdk_wayland_display_get_wl_display(gdk_display);
  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &registry_listener, this);
  wl_display_roundtrip(wl_display);

  if (layer_shell == nullptr)
    {
      TRACE_MSG("zwlr-layer-shell-v1 protocol unsupported");
      spdlog::warn("Your Wayland compositor does not support the wlr layer shell protocol. Workrave will not be able to "
                   "properly position its break windows.");
      return false;
    }

  TRACE_MSG("zwlr-layer-shell-v1 protocol supported");
  return true;
}

void
WaylandWindowManager::registry_global(void *data,
                                      struct wl_registry *registry,
                                      uint32_t id,
                                      const char *interface,
                                      uint32_t version)
{
  TRACE_ENTRY();
  TRACE_MSG("interface: {} {}", interface, version);
  auto *self = static_cast<WaylandWindowManager *>(data);
  if (g_strcmp0(zwlr_layer_shell_v1_interface.name, interface) == 0)
    {
      if (self->layer_shell != nullptr)
        {
          zwlr_layer_shell_v1_destroy(self->layer_shell);
        }

      self->layer_shell = static_cast<zwlr_layer_shell_v1 *>(
        wl_registry_bind(self->wl_registry,
                         id,
                         &zwlr_layer_shell_v1_interface,
                         MIN((uint32_t)zwlr_layer_shell_v1_interface.version, version)));
      self->layer_shell_id = id;
    }
}

void
WaylandWindowManager::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
  TRACE_ENTRY();
  auto *self = static_cast<WaylandWindowManager *>(data);
  if (self->layer_shell != nullptr && self->layer_shell_id == id)
    {
      TRACE_MSG("zwlr-layer-shell-v1 withdrawn");
      zwlr_layer_shell_v1_destroy(self->layer_shell);
      self->layer_shell = nullptr;
      self->layer_shell_id = 0;
    }
}

auto
WaylandWindowManager::init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus)
  -> std::shared_ptr<LayerSurface>
{
  TRACE_ENTRY();
  if (layer_shell == nullptr)
    {
      return {};
    }

  // Ownership stays with the caller: the manager is shared by every break and
  // prelude window, so it must not be able to tear down another window's surface.
  return std::make_shared<LayerSurface>(layer_shell, gtk_window, monitor, keyboard_focus);
}

LayerSurface::LayerSurface(struct zwlr_layer_shell_v1 *layer_shell,
                           Gtk::Widget &gtk_window,
                           Glib::RefPtr<Gdk::Monitor> monitor,
                           bool keyboard_focus)
  : layer_shell(layer_shell)
  , gtk_window(gtk_window.gobj())
  , keyboard_focus(keyboard_focus)

{
  TRACE_ENTRY();
  auto *gdk_display = gdk_display_get_default();
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition,cppcoreguidelines-pro-type-cstyle-cast)
  if (!GDK_IS_WAYLAND_DISPLAY(gdk_display))
    {
      TRACE_MSG("not running on wayland");
      return;
    }

  display = gdk_wayland_display_get_wl_display(gdk_display);

  gtk_widget_realize(gtk_window.gobj());
  auto window = gtk_window.get_window();
  wl_output *output = gdk_wayland_monitor_get_wl_output(monitor->gobj());

  gdk_wayland_window_set_use_custom_surface(window->gobj());

  auto *surface = gdk_wayland_window_get_wl_surface(window->gobj());
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
                                                        surface,
                                                        output,
                                                        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
                                                        "workrave");

  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   static_cast<uint32_t>(ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
                                     | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
  zwlr_layer_surface_v1_set_margin(layer_surface, 0, 0, 0, 0);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface,
                                                   keyboard_focus ? ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE
                                                                  : ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, this);

  wl_surface_commit(surface);
  wl_display_roundtrip(display);
}

LayerSurface::~LayerSurface()
{
  TRACE_ENTRY();
  if (layer_surface != nullptr)
    {
      zwlr_layer_surface_v1_destroy(layer_surface);
      layer_surface = nullptr;
    }
}

void
LayerSurface::layer_surface_configure(void *data,
                                      struct zwlr_layer_surface_v1 *surface,
                                      // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                      uint32_t serial,
                                      uint32_t width,
                                      uint32_t height)
{
  TRACE_ENTRY_PAR(width, height);
  auto *self = static_cast<LayerSurface *>(data);

  zwlr_layer_surface_v1_ack_configure(surface, serial);

  // The compositor decides the size of a layer surface anchored to all four
  // edges. Adopt it, otherwise the committed buffer does not match the size
  // that was just acknowledged. Do not roundtrip here: this runs inside a
  // Wayland event callback, and GDK flushes the surface itself.
  if (width > 0 && height > 0 && self->gtk_window != nullptr)
    {
      gtk_widget_set_size_request(self->gtk_window, static_cast<int>(width), static_cast<int>(height));
      gtk_window_resize(GTK_WINDOW(self->gtk_window), static_cast<int>(width), static_cast<int>(height));
    }
}

void
LayerSurface::layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
  TRACE_ENTRY();
}
