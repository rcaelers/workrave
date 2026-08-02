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

#include <gdk/gdk.h>
#if GDK_MAJOR_VERSION >= 4
#  include <gdk/wayland/gdkwayland.h>
#else
#  include <gdk/gdkwayland.h>
#endif

#include "debug.hh"

#if GTK_CHECK_VERSION(4, 0, 0) && defined(HAVE_GTK4_LAYER_SHELL)

bool
WaylandWindowManager::init()
{
  TRACE_ENTRY();
  return gtk_layer_is_supported();
}

void
WaylandWindowManager::setup_surface(Gtk::Window &gtk_window, const Glib::RefPtr<Gdk::Monitor> &monitor, bool keyboard_focus)
{
  TRACE_ENTRY();
  auto *window = gtk_window.gobj();

  gtk_layer_init_for_window(window);
  gtk_layer_set_namespace(window, "workrave");
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_OVERLAY);
  gtk_layer_set_keyboard_mode(window,
                              keyboard_focus ? GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE : GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
  if (monitor)
    {
      gtk_layer_set_monitor(window, monitor->gobj());
    }

  // Anchored to every edge: covers the whole output. GTK's own size
  // negotiation resizes the window to what the compositor grants, so unlike
  // the raw-protocol path there is no manual configure/resize step.
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
  gtk_layer_set_exclusive_zone(window, -1);
}

#elif GTK_CHECK_VERSION(4, 0, 0)

bool
WaylandWindowManager::init()
{
  TRACE_ENTRY();
  // No gtk4-layer-shell, and GTK4 alone has no supported way to hand a
  // surface to the layer shell (see WaylandWindowManager.hh), so there is no
  // point binding the protocol.
  return false;
}

auto
WaylandWindowManager::init_surface(Gtk::Widget & /*gtk_window*/, Glib::RefPtr<Gdk::Monitor> /*monitor*/, bool /*keyboard_focus*/)
  -> std::shared_ptr<WaylandLayerSurface>
{
  return {};
}

#else // GTK3

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

  return layer_shell.init(gdk_wayland_display_get_wl_display(gdk_display));
}

auto
WaylandWindowManager::init_surface(Gtk::Widget &gtk_window, Glib::RefPtr<Gdk::Monitor> monitor, bool keyboard_focus)
  -> std::shared_ptr<WaylandLayerSurface>
{
  TRACE_ENTRY();
  if (!layer_shell.is_supported() || !monitor)
    {
      return {};
    }

  auto *gdk_display = gdk_display_get_default();
  // NOLINTNEXTLINE(bugprone-assignment-in-if-condition,cppcoreguidelines-pro-type-cstyle-cast)
  if (!GDK_IS_WAYLAND_DISPLAY(gdk_display))
    {
      TRACE_MSG("not running on wayland");
      return {};
    }

  gtk_widget_realize(gtk_window.gobj());
  auto window = gtk_window.get_window();
  if (!window)
    {
      return {};
    }

  // Stops GDK from turning the surface into an xdg_toplevel of its own, so the
  // layer shell can take it over instead.
  gdk_wayland_window_set_use_custom_surface(window->gobj());

  auto *widget = gtk_window.gobj();
  return layer_shell.create_surface(gdk_wayland_window_get_wl_surface(window->gobj()),
                                    gdk_wayland_monitor_get_wl_output(monitor->gobj()),
                                    keyboard_focus,
                                    [widget](int width, int height) {
                                      gtk_widget_set_size_request(widget, width, height);
                                      if (GTK_IS_WINDOW(widget))
                                        {
                                          gtk_window_resize(GTK_WINDOW(widget), width, height);
                                        }
                                    });
}

#endif
