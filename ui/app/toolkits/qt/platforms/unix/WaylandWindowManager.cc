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

#include <wayland-client-protocol.h>
#include <memory>

#include "spdlog/spdlog.h"
#include "debug.hh"

#include <QWidget>
#include <QWindow>
#include <QScreen>

#include <qguiapplication_platform.h>
#include <QtGui/qpa/qplatformwindow_p.h>
#include <QtGui/qpa/qplatformscreen.h>
#include <QtGui/qpa/qplatformnativeinterface.h>

#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandsurface_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

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

  auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
  auto *wl_display = app->display();

  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &registry_listener, this);
  wl_display_roundtrip(wl_display);

  if (layer_shell == nullptr)
    {
      TRACE_MSG("zwlr-layer-surface protocol unsupported");
      spdlog::warn(
        "Your Wayland compositor does not support the wlr layer shell protocol. Workrave will not be able to properly position its break windows.");
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
  if (strcmp(zwlr_layer_shell_v1_interface.name, interface) == 0)
    {
      if (self->layer_shell != nullptr)
        {
          zwlr_layer_shell_v1_destroy(self->layer_shell);
        }

      self->layer_shell = static_cast<zwlr_layer_shell_v1 *>(
        wl_registry_bind(self->wl_registry,
                         id,
                         &zwlr_layer_shell_v1_interface,
                         std::min((uint32_t)zwlr_layer_shell_v1_interface.version, version)));
    }
}

void
WaylandWindowManager::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
}

void
WaylandWindowManager::init_surface(QWidget *window, QScreen *screen, bool keyboard_focus)
{
  TRACE_ENTRY();
  if (layer_shell != nullptr)
    {
      auto layer = std::make_shared<LayerSurface>(layer_shell, window, screen, keyboard_focus);
      surfaces.push_back(layer);
    }
}

void
WaylandWindowManager::clear_surfaces()
{
  TRACE_ENTRY();
  surfaces.clear();
}

LayerSurface::LayerSurface(struct zwlr_layer_shell_v1 *layer_shell, QWidget *window, QScreen *screen, bool keyboard_focus)
  : layer_shell(layer_shell)
  , keyboard_focus(keyboard_focus)

{
  TRACE_ENTRY();
  auto *wayland_window = window->windowHandle()->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
  auto *wayland_screen = screen->nativeInterface<QtWaylandClient::QWaylandScreen>();

  auto *output = wayland_screen->output();
  auto *surface = wayland_window->surface();
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
  zwlr_layer_surface_v1_destroy(this->layer_surface);
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
