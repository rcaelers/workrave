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

#include "debug.hh"

static const struct wl_registry_listener registry_listener = {
  .global = WaylandWindowManager::registry_global,
  .global_remove = WaylandWindowManager::registry_global_remove,
};

WaylandWindowManager::~WaylandWindowManager()
{
  if (layer_shell != nullptr)
    {
      zwlr_layer_shell_v1_destroy(layer_shell);
    }
  wl_registry_destroy(wl_registry);
  TRACE_ENTRY();
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
      TRACE_MSG("zwlr-layer-surface protocol unsupported");
      return false;
    }

  TRACE_MSG("ext-idle-notify-v1 protocol supported");
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
    }
}

void
WaylandWindowManager::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
}

void
WaylandWindowManager::init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus)
{
  TRACE_ENTRY();
  if (layer_shell != nullptr)
    {
      auto layer = std::make_shared<LayerSurface>(layer_shell, gtk_window, monitor, keyboard_focus);
      surfaces.push_back(layer);
    }
}

void
WaylandWindowManager::clear_surfaces()
{
  TRACE_ENTRY();
  surfaces.clear();
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
  gtk_widget_realize(gtk_window.gobj());
  auto window = gtk_window.get_window();
  wl_output *output = gdk_wayland_monitor_get_wl_output(monitor->gobj());

  auto *gdk_display = gdk_display_get_default();
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition,cppcoreguidelines-pro-type-cstyle-cast)
  if (!GDK_IS_WAYLAND_DISPLAY(gdk_display))
    {
      return;
    }

  display = gdk_wayland_display_get_wl_display(gdk_display);

  gtk_widget_realize(gtk_window.gobj());
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
  zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface, keyboard_focus ? TRUE : FALSE);
  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, this);

  wl_surface_commit(surface);
  wl_display_roundtrip(display);
}

void
LayerSurface::layer_surface_configure(void *data,
                                      struct zwlr_layer_surface_v1 *surface,
                                      // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                      uint32_t serial,
                                      uint32_t width,
                                      uint32_t height)
{
  TRACE_ENTRY();
  auto *self = static_cast<LayerSurface *>(data);
  zwlr_layer_surface_v1_ack_configure(surface, serial);
  wl_display_roundtrip(self->display);
}

void
LayerSurface::layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
  TRACE_ENTRY();
}
