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

#include "debug.hh"
#include "IInputMonitorListener.hh"
#include "Diagnostics.hh"

using namespace std;

MutterInputMonitor::MutterInputMonitor()
{
  monitor_thread = new Thread(this);
  g_mutex_init(&mutex);
  g_cond_init(&cond);
}

MutterInputMonitor::~MutterInputMonitor()
{
  if (monitor_thread != NULL)
    {
      monitor_thread->wait();
      delete monitor_thread;
    }

  g_mutex_clear(&mutex);
  g_cond_clear(&cond);
}

bool
MutterInputMonitor::init()
{
  TRACE_ENTER("MutterInputMonitor::init");

  bool result = init_idle_monitor();

  if (result)
    {
      init_inhibitors();
      init_service_monitor();
    }

  TRACE_EXIT();
  return result;
}

bool
MutterInputMonitor::init_idle_monitor()
{
  TRACE_ENTER("MutterInputMonitor::init");
  GError *error = NULL;
  bool result   = true;

  idle_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                             G_DBUS_PROXY_FLAGS_NONE,
                                             NULL,
                                             "org.gnome.Mutter.IdleMonitor",
                                             "/org/gnome/Mutter/IdleMonitor/Core",
                                             "org.gnome.Mutter.IdleMonitor",
                                             NULL,
                                             &error);
  if (error == NULL)
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
          monitor_thread->start();
        }
    }
  else
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
      result = false;
    }

  TRACE_EXIT();
  return result;
}

void
MutterInputMonitor::init_inhibitors()
{
  TRACE_ENTER("MutterInputMonitor::monitor_inhibitors");

  session_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,
                                                "org.gnome.SessionManager",
                                                "/org/gnome/SessionManager",
                                                "org.gnome.SessionManager",
                                                NULL,
                                                NULL);
  if (session_proxy != NULL)
    {
      g_signal_connect(session_proxy, "g-properties-changed", G_CALLBACK(on_session_manager_property_changed), this);

      GVariant *v     = g_dbus_proxy_get_cached_property(session_proxy, "InhibitedActions");
      inhibited       = (g_variant_get_uint32(v) & GSM_INHIBITOR_FLAG_IDLE) != 0;
      trace_inhibited = inhibited;
      TRACE_MSG("Inhibited:" << g_variant_get_uint32(v) << " " << inhibited);
      g_variant_unref(v);
    }
  TRACE_EXIT();
}

void
MutterInputMonitor::on_bus_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data)
{
  TRACE_ENTER_MSG("MutterInputMonitor::on_bus_name_appeared", name);
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
  TRACE_EXIT();
}

void
MutterInputMonitor::on_bus_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void)connection;
  (void)name;
  (void)user_data;
}

void
MutterInputMonitor::init_service_monitor()
{
  TRACE_ENTER("MutterInputMonitor::init_service_monitor");
  watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION, "org.gnome.Mutter.IdleMonitor", G_BUS_NAME_WATCHER_FLAGS_NONE, on_bus_name_appeared, on_bus_name_vanished, this, nullptr);
  TRACE_EXIT();
}

bool
MutterInputMonitor::register_active_watch()
{
  TRACE_ENTER("MutterInputMonitor::register_active_watch");
  GError *error   = NULL;
  GVariant *reply = g_dbus_proxy_call_sync(idle_proxy, "AddUserActiveWatch", NULL, G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);

  if (error == NULL)
    {
      guint watch = 0;
      g_variant_get(reply, "(u)", &watch);
      watch_active = active;
      g_variant_unref(reply);
    }
  else
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }
  TRACE_EXIT();
  return error == NULL;
}

void
MutterInputMonitor::register_active_watch_async()
{
  TRACE_ENTER("MutterInputMonitor::register_active_watch_aync");
  g_dbus_proxy_call(idle_proxy, "AddUserActiveWatch", NULL, G_DBUS_CALL_FLAGS_NONE, 10000, NULL, on_register_active_watch_reply, this);
  TRACE_EXIT();
}

void
MutterInputMonitor::on_register_active_watch_reply(GObject *object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTER("MutterInputMonitor::on_register_active_watch_reply");
  GError *error            = NULL;
  GDBusProxy *proxy        = G_DBUS_PROXY(object);
  MutterInputMonitor *self = (MutterInputMonitor *)user_data;

  GVariant *params = g_dbus_proxy_call_finish(proxy, res, &error);
  if (error)
    {
      TRACE_MSG("Error: " << error->message);
      g_clear_error(&error);
      return;
    }

  guint watch = 0;
  g_variant_get(params, "(u)", &watch);
  self->watch_active = watch;
  g_variant_unref(params);
}

bool
MutterInputMonitor::unregister_active_watch()
{
  TRACE_ENTER("MutterInputMonitor::unregister_active_watch");
  GError *error = NULL;
  if (watch_active != 0u)
    {
      GVariant *result = g_dbus_proxy_call_sync(idle_proxy, "RemoveWatch", g_variant_new("(u)", watch_active), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);
      if (error == NULL)
        {
          g_variant_unref(result);
          watch_active = 0;
        }
      else
        {
          TRACE_MSG("Error: " << error->message);
          g_error_free(error);
        }
    }
  TRACE_EXIT();
  return error == NULL;
}

void
MutterInputMonitor::unregister_active_watch_async()
{
  TRACE_ENTER("MutterInputMonitor::unregister_active_watch_async");
  if (watch_active != 0u)
    {
      g_dbus_proxy_call(idle_proxy,
                        "RemoveWatch",
                        g_variant_new("(u)", watch_active),
                        G_DBUS_CALL_FLAGS_NONE,
                        10000,
                        NULL,
                        on_unregister_active_watch_reply,
                        this);
    }
  TRACE_EXIT();
}

void
MutterInputMonitor::on_unregister_active_watch_reply(GObject *object, GAsyncResult *res, gpointer user_data)
{
  TRACE_ENTER("MutterInputMonitor::on_unregister_active_watch_reply");
  GError *error            = NULL;
  GDBusProxy *proxy        = G_DBUS_PROXY(object);
  MutterInputMonitor *self = (MutterInputMonitor *)user_data;

  g_dbus_proxy_call_finish(proxy, res, &error);
  if (error)
    {
      TRACE_MSG("Error: " << error->message);
      g_clear_error(&error);
      return;
    }
  self->watch_active = 0;
  TRACE_EXIT();
}

bool
MutterInputMonitor::register_idle_watch()
{
  TRACE_ENTER("MutterInputMonitor::register_idle_watch");
  GError *error = NULL;
  GVariant *reply =
    g_dbus_proxy_call_sync(idle_proxy, "AddIdleWatch", g_variant_new("(t)", 500), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);

  if (error == NULL)
    {
      guint watch = 0;
      ;
      g_variant_get(reply, "(u)", &watch);
      watch_idle = watch;
      g_variant_unref(reply);
    }
  else
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
    }
  TRACE_EXIT();
  return error == NULL;
}

bool
MutterInputMonitor::unregister_idle_watch()
{
  TRACE_ENTER("MutterInputMonitor::unregister_idle_watch");
  GError *error = NULL;
  if (watch_idle != 0u)
    {
      GVariant *result = g_dbus_proxy_call_sync(
        idle_proxy, "RemoveWatch", g_variant_new("(u)", watch_idle), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);
      if (error == NULL)
        {
          g_variant_unref(result);
          watch_idle = 0;
        }
      else
        {
          TRACE_MSG("Error: " << error->message);
          g_error_free(error);
        }
    }
  TRACE_EXIT();
  return error == NULL;
}

void
MutterInputMonitor::terminate()
{
  unregister_idle_watch();
  unregister_active_watch();

  g_mutex_lock(&mutex);
  abort = true;
  g_cond_broadcast(&cond);
  g_mutex_unlock(&mutex);

  monitor_thread->wait();
  monitor_thread = NULL;
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

  MutterInputMonitor *self = (MutterInputMonitor *)user_data;

  if (g_strcmp0(signal_name, "WatchFired") == 0)
    {
      guint handlerID;
      g_variant_get(parameters, "(u)", &handlerID);

      Diagnostics::instance().log("mutter: watch fired");
      if (handlerID == self->watch_active)
        {
          self->unregister_active_watch_async();
          self->active       = true;
          self->trace_active = true;
        }
      else if (handlerID == self->watch_idle)
        {
          self->register_active_watch_async();
          self->active       = false;
          self->trace_active = false;
        }
      else
        {
          Diagnostics::instance().log("mutter: unknown handler ID");
        }
    }
}

void
MutterInputMonitor::on_session_manager_property_changed(GDBusProxy *session,
                                                        GVariant *changed,
                                                        char **invalidated,
                                                        gpointer user_data)
{
  TRACE_ENTER("MutterInputMonitor::on_session_manager_property_changed");
  (void)session;
  (void)invalidated;

  MutterInputMonitor *self = (MutterInputMonitor *)user_data;

  GVariant *v = g_variant_lookup_value(changed, "InhibitedActions", G_VARIANT_TYPE_UINT32);
  if (v != NULL)
    {
      self->inhibited       = g_variant_get_uint32(v) & GSM_INHIBITOR_FLAG_IDLE;
      self->trace_inhibited = self->inhibited;
      TRACE_MSG("Inhibited:" << g_variant_get_uint32(v));
      g_variant_unref(v);
    }
  TRACE_EXIT();
}

void
MutterInputMonitor::run()
{
  TRACE_ENTER("MutterInputMonitor::run");

  g_mutex_lock(&mutex);
  while (!abort)
    {
      bool local_active = active;

      if (inhibited)
        {
          GError *error = NULL;
          guint64 idletime;
          GVariant *reply = g_dbus_proxy_call_sync(idle_proxy, "GetIdletime", NULL, G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);
          if (error == NULL)
            {

              g_variant_get(reply, "(t)", &idletime);
              g_variant_unref(reply);
              Diagnostics::instance().log("mutter: " + std::to_string(idletime));
              local_active = idletime < 1000;
            }
          else
            {
              TRACE_MSG("Error: " << error->message);
              g_error_free(error);
            }
        }

      if (local_active)
        {
          /* Notify the activity monitor */
          fire_action();
        }

      gint64 end_time = g_get_monotonic_time() + G_TIME_SPAN_SECOND;
      g_cond_wait_until(&cond, &mutex, end_time);
    }
  g_mutex_unlock(&mutex);

  TRACE_EXIT();
}
