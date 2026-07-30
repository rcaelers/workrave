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
