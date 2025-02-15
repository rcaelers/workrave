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

#include "QtWaylandInputMonitor.hh"

#include <memory>
#include <wayland-client-protocol.h>
#include <QGuiApplication>

#include "wayland-ext-idle-notify-v1-client-protocol.h"
#include "debug.hh"

static const struct wl_registry_listener registry_listener = {
  .global = QtWaylandInputMonitor::registry_global,
  .global_remove = QtWaylandInputMonitor::registry_global_remove,
};

static const struct ext_idle_notification_v1_listener idle_notification_listener = {
  .idled = QtWaylandInputMonitor::notification_idled,
  .resumed = QtWaylandInputMonitor::notification_resumed,
};

QtWaylandInputMonitor::~QtWaylandInputMonitor()
{
  TRACE_ENTRY();
  if (monitor_thread)
    {
      monitor_thread->join();
    }
}

bool
QtWaylandInputMonitor::init()
{
  TRACE_ENTRY();

  auto *app = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
  auto *wl_display = app->display();
  auto *wl_seat = app->seat();

  wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &registry_listener, this);
  wl_display_roundtrip(wl_display);

  if (wl_notifier == nullptr)
    {
      TRACE_MSG("ext-idle-notify-v1 protocol unsupported");
      return false;
    }

  auto *wl_notification = ext_idle_notifier_v1_get_idle_notification(wl_notifier, timeout, wl_seat);

  ext_idle_notification_v1_add_listener(wl_notification, &idle_notification_listener, this);
  monitor_thread = std::make_shared<std::thread>([this] { run(); });

  TRACE_MSG("ext-idle-notify-v1 protocol supported");
  return true;
}

void
QtWaylandInputMonitor::terminate()
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
QtWaylandInputMonitor::registry_global(void *data,
                                       struct wl_registry *registry,
                                       uint32_t id,
                                       const char *interface,
                                       uint32_t version)
{
  TRACE_ENTRY();
  auto *self = static_cast<QtWaylandInputMonitor *>(data);
  if (strcmp(ext_idle_notifier_v1_interface.name, interface) == 0)
    {
      if (self->wl_notifier != nullptr)
        {
          ext_idle_notifier_v1_destroy(self->wl_notifier);
        }

      self->wl_notifier = static_cast<ext_idle_notifier_v1 *>(
        wl_registry_bind(self->wl_registry,
                         id,
                         &ext_idle_notifier_v1_interface,
                         std::min((uint32_t)ext_idle_notifier_v1_interface.version, version)));
    }
}

void
QtWaylandInputMonitor::registry_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
}

void
QtWaylandInputMonitor::notification_idled(void *data, struct ext_idle_notification_v1 *notification)
{
  auto *self = static_cast<QtWaylandInputMonitor *>(data);
  self->idle = true;
}

void
QtWaylandInputMonitor::notification_resumed(void *data, struct ext_idle_notification_v1 *notification)
{
  auto *self = static_cast<QtWaylandInputMonitor *>(data);
  self->idle = false;
}

void
QtWaylandInputMonitor::run()
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
