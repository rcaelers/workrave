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

#ifndef WAYLANDINPUTMONITOR_HH
#define WAYLANDINPUTMONITOR_HH

#include "InputMonitor.hh"

#include <atomic>
#include <condition_variable>
#include <thread>

#include <gdk/gdkwayland.h>
#include <gio/gio.h>

class WaylandInputMonitor : public InputMonitor
{
public:
  WaylandInputMonitor() = default;
  ~WaylandInputMonitor() override;

  bool init() override;
  void terminate() override;
  void run();

public:
  static void registry_global(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
  static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t id);
  static void notification_idled(void *data, struct ext_idle_notification_v1 *notification);
  static void notification_resumed(void *data, struct ext_idle_notification_v1 *notification);

private:
  static constexpr int timeout = 1000;

private:
  bool abort{false};
  std::shared_ptr<std::thread> monitor_thread;
  std::mutex mutex;
  std::condition_variable cond{};
  std::atomic<bool> idle{false};
  struct wl_registry *wl_registry{};
  struct ext_idle_notifier_v1 *wl_notifier{};
};

#endif // WAYLANDINPUTMONITOR_HH
