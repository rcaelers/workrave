// Copyright (C) 2016 Rob Caelers <robc@krandor.nl>
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

#ifndef MUTTERINPUTMONITOR_HH
#define MUTTERINPUTMONITOR_HH

#include "InputMonitor.hh"

#include <gio/gio.h>
#include <atomic>

#include "Runnable.hh"
#include "Thread.hh"
#include "Diagnostics.hh"

class MutterInputMonitor
  : public InputMonitor
  , public Runnable
{
public:
  MutterInputMonitor();

  //! Destructor.
  ~MutterInputMonitor() override;

  //! Initialize
  bool init() override;

  //! Terminate the monitor.
  void terminate() override;

private:
  static void
  on_idle_monitor_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
  static void on_session_manager_property_changed(GDBusProxy *session, GVariant *changed, char **invalidated, gpointer user_data);

  static void on_register_active_watch_reply(GObject *source_object, GAsyncResult *res, gpointer user_data);
  static void on_unregister_active_watch_reply(GObject *source_object, GAsyncResult *res, gpointer user_data);

  //! The monitor's execution thread.
  void run() override;

  bool register_active_watch();
  bool unregister_active_watch();
  void register_active_watch_async();
  void unregister_active_watch_async();
  bool register_idle_watch();
  bool unregister_idle_watch();

  bool init_idle_monitor();
  void init_inhibitors();

private:
  static const int GSM_INHIBITOR_FLAG_IDLE = 8;

  GDBusProxy *idle_proxy = nullptr;
  GDBusProxy *session_proxy = nullptr;
  std::atomic<bool> active{false};
  std::atomic<bool> inhibited{false};
  TracedField<bool> trace_active{"monitor.mutter.active", false};
  TracedField<bool> trace_inhibited{"monitor.inhibited", false};
  TracedField<guint> watch_active{"monitor.mutter.watch_active", 0};
  TracedField<guint> watch_idle{"monitor.mutter.watch_idle", 0};

  bool abort = false;
  Thread *monitor_thread = nullptr;
  GMutex mutex{};
  GCond cond{};
};

#endif // MUTTERINPUTMONITOR_HH
