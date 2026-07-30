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

#ifndef WAYLANDLAYERSHELL_HH
#define WAYLANDLAYERSHELL_HH

#include <cstdint>
#include <functional>
#include <memory>

#include <wayland-client-core.h>

#include "wlr-layer-shell-unstable-v1-client.h"

//! A zwlr_layer_surface_v1 covering a single output.
/*!
 *  Toolkit neutral: it only needs the raw Wayland handles, so both the gtkmm
 *  and the Qt toolkit can drive it after extracting those from their own
 *  window abstractions.
 */
class WaylandLayerSurface
{
public:
  //! Reports the size the compositor picked for the surface.
  using ConfigureFunc = std::function<void(int width, int height)>;

  WaylandLayerSurface(struct zwlr_layer_shell_v1 *layer_shell,
                      struct wl_display *display,
                      struct wl_surface *surface,
                      struct wl_output *output,
                      bool keyboard_focus,
                      ConfigureFunc on_configure);
  ~WaylandLayerSurface();

  WaylandLayerSurface(const WaylandLayerSurface &) = delete;
  WaylandLayerSurface &operator=(const WaylandLayerSurface &) = delete;
  WaylandLayerSurface(WaylandLayerSurface &&) = delete;
  WaylandLayerSurface &operator=(WaylandLayerSurface &&) = delete;

private:
  static void on_configure(void *data,
                           struct zwlr_layer_surface_v1 *surface,
                           uint32_t serial,
                           uint32_t width,
                           uint32_t height);
  static void on_closed(void *data, struct zwlr_layer_surface_v1 *surface);

  struct zwlr_layer_surface_v1 *layer_surface{};
  ConfigureFunc configure_func;

  static const struct zwlr_layer_surface_v1_listener surface_listener;
};

//! Binds zwlr_layer_shell_v1.
/*!
 *  This is the only Wayland protocol that lets a client place a surface on a
 *  specific output without going fullscreen. Mutter does not implement it, so
 *  init() failing is an expected outcome on GNOME rather than an error.
 */
class WaylandLayerShell
{
public:
  WaylandLayerShell() = default;
  ~WaylandLayerShell();

  WaylandLayerShell(const WaylandLayerShell &) = delete;
  WaylandLayerShell &operator=(const WaylandLayerShell &) = delete;
  WaylandLayerShell(WaylandLayerShell &&) = delete;
  WaylandLayerShell &operator=(WaylandLayerShell &&) = delete;

  //! Binds the protocol. Returns false when the compositor does not offer it.
  bool init(struct wl_display *display);

  bool is_supported() const
  {
    return layer_shell != nullptr;
  }

  //! Creates a surface anchored to every edge of @p output.
  /*!
   *  Ownership is the caller's: a single shell is shared by every break and
   *  prelude window, so it must not be able to tear down another window's
   *  surface. Returns nullptr when the protocol is unavailable.
   */
  auto create_surface(struct wl_surface *surface,
                      struct wl_output *output,
                      bool keyboard_focus,
                      WaylandLayerSurface::ConfigureFunc on_configure) -> std::shared_ptr<WaylandLayerSurface>;

private:
  static void registry_global(void *data,
                              struct wl_registry *registry,
                              uint32_t id,
                              const char *interface,
                              uint32_t version);
  static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t id);

  struct wl_display *display{};
  struct wl_registry *wl_registry{};
  struct zwlr_layer_shell_v1 *layer_shell{};
  uint32_t layer_shell_id{};

  static const struct wl_registry_listener registry_listener;
};

#endif // WAYLANDLAYERSHELL_HH
