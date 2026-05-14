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

#include "WaylandInputMonitor.hh"

#include <memory>
#include <gdk/gdkwayland.h>

#include "ext-idle-notify-v1-client.h"
#include "debug.hh"

static const struct wl_registry_listener registry_listener = {
  .global = WaylandInputMonitor::registry_global,
  .global_remove = WaylandInputMonitor::registry_global_remove,
};

static const struct ext_idle_notification_v1_listener idle_notification_listener = {
  .idled = WaylandInputMonitor::notification_idled,
  .resumed = WaylandInputMonitor::notification_resumed,
};

WaylandInputMonitor::~WaylandInputMonitor()
{
  TRACE_ENTRY();
  if (monitor_thread)
    {
      monitor_thread->join();
    }
}

bool
WaylandInputMonitor::init()
{
  TRACE_ENTRY();
  auto *gdk_display = gdk_display_get_default();
  if (gdk_display == nullptr)
    {
      TRACE_MSG("no default display");
      return false;
    }

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

  if (wl_notifier == nullptr)
    {
      TRACE_MSG("ext-idle-notify-v1 protocol unsupported");
      return false;
    }

  auto *wl_seat = gdk_wayland_seat_get_wl_seat(gdk_display_get_default_seat(gdk_display));

  const auto wl_ntfr_ver = ext_idle_notifier_v1_get_version(wl_notifier);
  const auto wl_ntfr_ver_w_input_idle = decltype(wl_ntfr_ver){2};

  ext_idle_notification_v1 *wl_notification = nullptr;
  if (wl_ntfr_ver < wl_ntfr_ver_w_input_idle)
    {
      TRACE_MSG("Falling back to version of ext-idle-notify-v1 protocol that "
		"does not support ignoring idle inhibitors");

      wl_notification = ext_idle_notifier_v1_get_idle_notification(wl_notifier,
								   timeout, wl_seat);
    }
  else
    {
      TRACE_MSG("Using version of ext-idle-notify-v1 protocol that "
		"supports ignoring idle inhibitors");
	
      wl_notification = ext_idle_notifier_v1_get_input_idle_notification(wl_notifier,
									 timeout, wl_seat);
    }
  
  ext_idle_notification_v1_add_listener(wl_notification, &idle_notification_listener, this);
  monitor_thread = std::make_shared<std::thread>([this] { run(); });

  TRACE_MSG("ext-idle-notify-v1 protocol supported");
  return true;
}

void
WaylandInputMonitor::terminate()
{
  TRACE_ENTRY();
  mutex.lock();
  abort = true;
  cond.notify_all();
  mutex.unlock();

  if (monitor_thread)
    {
      monitor_thread->join();
    }

  if (wl_notifier != nullptr)
    {
      ext_idle_notifier_v1_destroy(wl_notifier);
    }
  wl_registry_destroy(wl_registry);
}

void
WaylandInputMonitor::registry_global(void *data,
                                     struct wl_registry *registry,
                                     uint32_t id,
                                     const char *interface,
                                     uint32_t version)
{
  TRACE_ENTRY();
  auto *self = static_cast<WaylandInputMonitor *>(data);
  if (g_strcmp0(ext_idle_notifier_v1_interface.name, interface) == 0)
    {
      if (self->wl_notifier != nullptr)
        {
          ext_idle_notifier_v1_destroy(self->wl_notifier);
        }

      self->wl_notifier = static_cast<ext_idle_notifier_v1 *>(
        wl_registry_bind(self->wl_registry,
                         id,
                         &ext_idle_notifier_v1_interface,
                         MIN((uint32_t)ext_idle_notifier_v1_interface.version, version)));
    }
}

void
WaylandInputMonitor::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
}

void
WaylandInputMonitor::notification_idled(void *data, struct ext_idle_notification_v1 *notification)
{
  auto *self = static_cast<WaylandInputMonitor *>(data);
  self->idle = true;
}

void
WaylandInputMonitor::notification_resumed(void *data, struct ext_idle_notification_v1 *notification)
{
  auto *self = static_cast<WaylandInputMonitor *>(data);
  self->idle = false;
}

void
WaylandInputMonitor::run()
{
  TRACE_ENTRY();
  {
    std::unique_lock lock(mutex);
    while (!abort)
      {
        if (!idle)
          {
            fire_action();
          }

        cond.wait_for(lock, std::chrono::milliseconds(timeout));
      }
  }
}
