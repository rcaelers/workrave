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

#include "WaylandLayerShell.hh"

#include <algorithm>
#include <cstring>
#include <utility>

#include <spdlog/spdlog.h>

#include "debug.hh"

const struct wl_registry_listener WaylandLayerShell::registry_listener = {
  .global = WaylandLayerShell::registry_global,
  .global_remove = WaylandLayerShell::registry_global_remove,
};

const struct zwlr_layer_surface_v1_listener WaylandLayerSurface::surface_listener = {
  .configure = WaylandLayerSurface::on_configure,
  .closed = WaylandLayerSurface::on_closed,
};

WaylandLayerShell::~WaylandLayerShell()
{
  TRACE_ENTRY();
  // Surfaces are owned by the windows and stay valid after the shell is
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
WaylandLayerShell::init(struct wl_display *display)
{
  TRACE_ENTRY();
  if (display == nullptr)
    {
      TRACE_MSG("no wayland display");
      return false;
    }

  this->display = display;
  wl_registry = wl_display_get_registry(display);
  if (wl_registry == nullptr)
    {
      return false;
    }

  wl_registry_add_listener(wl_registry, &registry_listener, this);
  wl_display_roundtrip(display);

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
WaylandLayerShell::registry_global(void *data,
                                   struct wl_registry *registry,
                                   uint32_t id,
                                   const char *interface,
                                   uint32_t version)
{
  TRACE_ENTRY();
  TRACE_MSG("interface: {} {}", interface, version);
  auto *self = static_cast<WaylandLayerShell *>(data);

  if (std::strcmp(zwlr_layer_shell_v1_interface.name, interface) == 0)
    {
      if (self->layer_shell != nullptr)
        {
          zwlr_layer_shell_v1_destroy(self->layer_shell);
        }

      self->layer_shell = static_cast<zwlr_layer_shell_v1 *>(
        wl_registry_bind(registry,
                         id,
                         &zwlr_layer_shell_v1_interface,
                         std::min(static_cast<uint32_t>(zwlr_layer_shell_v1_interface.version), version)));
      self->layer_shell_id = id;
    }
}

void
WaylandLayerShell::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
  TRACE_ENTRY();
  auto *self = static_cast<WaylandLayerShell *>(data);
  if (self->layer_shell != nullptr && self->layer_shell_id == id)
    {
      TRACE_MSG("zwlr-layer-shell-v1 withdrawn");
      zwlr_layer_shell_v1_destroy(self->layer_shell);
      self->layer_shell = nullptr;
      self->layer_shell_id = 0;
    }
}

auto
WaylandLayerShell::create_surface(struct wl_surface *surface,
                                  struct wl_output *output,
                                  bool keyboard_focus,
                                  WaylandLayerSurface::ConfigureFunc on_configure) -> std::shared_ptr<WaylandLayerSurface>
{
  TRACE_ENTRY();
  if (layer_shell == nullptr || surface == nullptr)
    {
      return {};
    }

  return std::make_shared<WaylandLayerSurface>(layer_shell, display, surface, output, keyboard_focus, std::move(on_configure));
}

WaylandLayerSurface::WaylandLayerSurface(struct zwlr_layer_shell_v1 *layer_shell,
                                         struct wl_display *display,
                                         struct wl_surface *surface,
                                         struct wl_output *output,
                                         bool keyboard_focus,
                                         ConfigureFunc on_configure)
  : configure_func(std::move(on_configure))
{
  TRACE_ENTRY();
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell,
                                                        surface,
                                                        output,
                                                        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
                                                        "workrave");
  if (layer_surface == nullptr)
    {
      return;
    }

  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   static_cast<uint32_t>(ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
                                     | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
  zwlr_layer_surface_v1_set_margin(layer_surface, 0, 0, 0, 0);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface,
                                                   keyboard_focus ? ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE
                                                                  : ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
  zwlr_layer_surface_v1_add_listener(layer_surface, &surface_listener, this);

  wl_surface_commit(surface);
  wl_display_roundtrip(display);
}

WaylandLayerSurface::~WaylandLayerSurface()
{
  TRACE_ENTRY();
  if (layer_surface != nullptr)
    {
      zwlr_layer_surface_v1_destroy(layer_surface);
      layer_surface = nullptr;
    }
}

void
WaylandLayerSurface::on_configure(void *data,
                                  struct zwlr_layer_surface_v1 *surface,
                                  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                  uint32_t serial,
                                  uint32_t width,
                                  uint32_t height)
{
  TRACE_ENTRY_PAR(width, height);
  auto *self = static_cast<WaylandLayerSurface *>(data);

  zwlr_layer_surface_v1_ack_configure(surface, serial);

  // The compositor decides the size of a surface anchored to all four edges.
  // Adopt it, otherwise the committed buffer does not match the size that was
  // just acknowledged. Do not roundtrip here: this runs inside a Wayland event
  // callback, and the toolkit flushes the surface itself.
  if (width > 0 && height > 0 && self->configure_func)
    {
      self->configure_func(static_cast<int>(width), static_cast<int>(height));
    }
}

void
WaylandLayerSurface::on_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
  TRACE_ENTRY();
}
