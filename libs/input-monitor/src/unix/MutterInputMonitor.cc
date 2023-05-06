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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MutterInputMonitor.hh"

#include <memory>

#include "debug.hh"
#include "utils/Diagnostics.hh"

using namespace std;

MutterInputMonitor::~MutterInputMonitor()
{
  if (watch_id != 0)
    {
      g_bus_unwatch_name(watch_id);
    }
  if (idle_proxy != nullptr)
    {
      g_object_unref(idle_proxy);
    }
  if (session_proxy != nullptr)
    {
      g_object_unref(session_proxy);
    }

  if (monitor_thread)
    {
      monitor_thread->join();
    }
}

bool
MutterInputMonitor::init()
{
  TRACE_ENTRY();
  bool result = init_idle_monitor();

  if (result)
    {
      init_inhibitors();
      init_service_monitor();
    }

  return result;
}

bool
MutterInputMonitor::init_idle_monitor()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  bool result = true;

  idle_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                             G_DBUS_PROXY_FLAGS_NONE,
                                             nullptr,
                                             "org.gnome.Mutter.IdleMonitor",
                                             "/org/gnome/Mutter/IdleMonitor/Core",
                                             "org.gnome.Mutter.IdleMonitor",
                                             nullptr,
                                             &error);
  if (error == nullptr)
    {
      g_signal_connect(idle_proxy, "g-signal", G_CALLBACK(on_idle_monitor_signal), this);

      result = register_active_watch();
      if (result)
        {
          result = register_idle_watch();
          if (!result)
            {
              unregister_active_watch();
            }
        }

      if (result)
        {
          monitor_thread = std::make_shared<std::thread>([this] { run(); });
        }
    }
  else
    {
      TRACE_MSG("Error: {}", error->message);
      g_error_free(error);
      result = false;
    }

  return result;
}

void
MutterInputMonitor::init_inhibitors()
{
  TRACE_ENTRY();
  session_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                nullptr,
                                                "org.gnome.SessionManager",
                                                "/org/gnome/SessionManager",
                                                "org.gnome.SessionManager",
                                                nullptr,
                                                nullptr);
  if (session_proxy != nullptr)
    {
      g_signal_connect(session_proxy, "g-properties-changed", G_CALLBACK(on_session_manager_property_changed), this);

      GVariant *v = g_dbus_proxy_get_cached_property(session_proxy, "InhibitedActions");
      inhibited = (g_variant_get_uint32(v) & GSM_INHIBITOR_FLAG_IDLE) != 0;
      trace_inhibited = inhibited;
      TRACE_MSG("Inhibited: {} {}", g_variant_get_uint32(v), inhibited);
      g_variant_unref(v);
    }
}

void
MutterInputMonitor::on_bus_name_appeared(GDBusConnection *connection,
                                         const gchar *name,
                                         const gchar *name_owner,
                                         gpointer user_data)
{
  TRACE_ENTRY_PAR(name);
  (void)connection;
  (void)name;
  (void)name_owner;
  auto *self = (MutterInputMonitor *)user_data;
  if (self->watch_active != 0u)
    {
      self->register_active_watch();
    }
  if (self->watch_idle != 0u)
    {
      self->register_idle_watch();
    }
}

void
MutterInputMonitor::init_service_monitor()
{
  TRACE_ENTRY();
  watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                              "org.gnome.Mutter.IdleMonitor",
                              G_BUS_NAME_WATCHER_FLAGS_NONE,
                              on_bus_name_appeared,
                              nullptr,
                              this,
                              nullptr);
}

bool
MutterInputMonitor::register_active_watch()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  GVariant
    *reply = g_dbus_proxy_call_sync(idle_proxy, "AddUserActiveWatch", nullptr, G_DBUS_CALL_FLAGS_NONE, 10000, nullptr, &error);

  if (error == nullptr)
    {
      guint watch = 0;
      g_variant_get(reply, "(u)", &watch);
      watch_active = watch;
      g_variant_unref(reply);
      Diagnostics::instance().log(fmt::format("mutter: active handler ID {}", watch_active));
    }
  else
    {
      TRACE_MSG("Error: {}", error->message);
      g_error_free(error);
    }
  return error == nullptr;
}

void
MutterInputMonitor::register_active_watch_async()
{
  TRACE_ENTRY();
  g_dbus_proxy_call(idle_proxy,
                    "AddUserActiveWatch",
                    nullptr,
                    G_DBUS_CALL_FLAGS_NONE,
                    10000,
                    nullptr,
                    on_register_active_watch_reply,
                    this);
}

void
MutterInputMonitor::on_register_active_watch_reply(GObject *object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTRY();
  GError *error = nullptr;
  GDBusProxy *proxy = G_DBUS_PROXY(object);
  auto *self = (MutterInputMonitor *)user_data;

  GVariant *params = g_dbus_proxy_call_finish(proxy, res, &error);
  if (error)
    {
      TRACE_MSG("Error: {}", error->message);
      g_clear_error(&error);
      return;
    }

  guint watch = 0;
  g_variant_get(params, "(u)", &watch);
  self->watch_active = watch;
  g_variant_unref(params);
  Diagnostics::instance().log(fmt::format("mutter: active handler ID async {}", self->watch_active));
}

bool
MutterInputMonitor::unregister_active_watch()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  if (watch_active != 0u)
    {
      GVariant *result = g_dbus_proxy_call_sync(idle_proxy,
                                                "RemoveWatch",
                                                g_variant_new("(u)", watch_active.get()),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                10000,
                                                nullptr,
                                                &error);
      if (error == nullptr)
        {
          g_variant_unref(result);
          watch_active = 0;
        }
      else
        {
          TRACE_MSG("Error: {}", error->message);
          g_error_free(error);
        }
    }
  return error == nullptr;
}

void
MutterInputMonitor::unregister_active_watch_async()
{
  TRACE_ENTRY();
  if (watch_active != 0u)
    {
      g_dbus_proxy_call(idle_proxy,
                        "RemoveWatch",
                        g_variant_new("(u)", watch_active.get()),
                        G_DBUS_CALL_FLAGS_NONE,
                        10000,
                        nullptr,
                        on_unregister_active_watch_reply,
                        this);
    }
}

void
MutterInputMonitor::on_unregister_active_watch_reply(GObject *object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTRY();
  GError *error = nullptr;
  GDBusProxy *proxy = G_DBUS_PROXY(object);
  auto *self = (MutterInputMonitor *)user_data;

  g_dbus_proxy_call_finish(proxy, res, &error);
  if (error)
    {
      TRACE_MSG("Error: {}", error->message);
      g_clear_error(&error);
      return;
    }
  self->watch_active = 0;
}

bool
MutterInputMonitor::register_idle_watch()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  GVariant *reply = g_dbus_proxy_call_sync(idle_proxy,
                                           "AddIdleWatch",
                                           g_variant_new("(t)", 500),
                                           G_DBUS_CALL_FLAGS_NONE,
                                           10000,
                                           nullptr,
                                           &error);

  if (error == nullptr)
    {
      guint watch = 0;
      g_variant_get(reply, "(u)", &watch);
      watch_idle = watch;
      g_variant_unref(reply);
      Diagnostics::instance().log(fmt::format("mutter: idle handler ID {}", watch_idle));
    }
  else
    {
      TRACE_MSG("Error: {}", error->message);
      g_error_free(error);
    }
  return error == nullptr;
}

bool
MutterInputMonitor::unregister_idle_watch()
{
  TRACE_ENTRY();
  GError *error = nullptr;
  if (watch_idle != 0u)
    {
      GVariant *result = g_dbus_proxy_call_sync(idle_proxy,
                                                "RemoveWatch",
                                                g_variant_new("(u)", watch_idle.get()),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                10000,
                                                nullptr,
                                                &error);
      if (error == nullptr)
        {
          g_variant_unref(result);
          watch_idle = 0;
        }
      else
        {
          TRACE_MSG("Error: {}", error->message);
          g_error_free(error);
        }
    }
  return error == nullptr;
}

void
MutterInputMonitor::terminate()
{
  unregister_idle_watch();
  unregister_active_watch();

  mutex.lock();
  abort = true;
  cond.notify_all();
  mutex.unlock();

  monitor_thread->join();
}

void
MutterInputMonitor::on_idle_monitor_signal(GDBusProxy *proxy,
                                           gchar *sender_name,
                                           gchar *signal_name,
                                           GVariant *parameters,
                                           gpointer user_data)
{
  (void)proxy;
  (void)sender_name;

  auto *self = (MutterInputMonitor *)user_data;

  if (g_strcmp0(signal_name, "WatchFired") == 0)
    {
      guint handlerID;
      g_variant_get(parameters, "(u)", &handlerID);

      Diagnostics::instance().log(fmt::format("mutter: watch fired handler ID {}", handlerID));
      if (handlerID == self->watch_active)
        {
          self->unregister_active_watch_async();
          self->active = true;
          self->trace_active = true;
        }
      else if (handlerID == self->watch_idle)
        {
          self->register_active_watch_async();
          self->active = false;
          self->trace_active = false;
        }
      else
        {
          Diagnostics::instance().log(fmt::format("mutter: unknown handler ID {}", handlerID));
        }
    }
}

void
MutterInputMonitor::on_session_manager_property_changed(GDBusProxy *session,
                                                        GVariant *changed,
                                                        char **invalidated,
                                                        gpointer user_data)
{
  TRACE_ENTRY();
  (void)session;
  (void)invalidated;

  auto *self = (MutterInputMonitor *)user_data;

  GVariant *v = g_variant_lookup_value(changed, "InhibitedActions", G_VARIANT_TYPE_UINT32);
  if (v != nullptr)
    {
      self->inhibited = g_variant_get_uint32(v) & GSM_INHIBITOR_FLAG_IDLE;
      self->trace_inhibited = self->inhibited;
      TRACE_MSG("Inhibited: {}", g_variant_get_uint32(v));
      g_variant_unref(v);
    }
}

void
MutterInputMonitor::run()
{
  TRACE_ENTRY();
  {
    std::unique_lock lock(mutex);
    while (!abort)
      {
        bool local_active = active;

        if (inhibited)
          {
            GError *error = nullptr;
            guint64 idletime;
            GVariant
              *reply = g_dbus_proxy_call_sync(idle_proxy, "GetIdletime", nullptr, G_DBUS_CALL_FLAGS_NONE, 10000, nullptr, &error);
            if (error == nullptr)
              {

                g_variant_get(reply, "(t)", &idletime);
                g_variant_unref(reply);
                Diagnostics::instance().log("mutter: " + std::to_string(idletime));
                local_active = idletime < 1000;
              }
            else
              {
                TRACE_MSG("Error: {}", error->message);
                g_error_free(error);
              }
          }

        if (local_active)
          {
            /* Notify the activity monitor */
            fire_action();
          }

        cond.wait_for(lock, std::chrono::milliseconds(1000));
      }
  }
}
