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
#include "config.h"
#endif

#include "MutterInputMonitor.hh"

#include "debug.hh"
#include "IInputMonitorListener.hh"

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
  GError *error = NULL;
  bool result = true;

  proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        "org.gnome.Mutter.IdleMonitor",
                                        "/org/gnome/Mutter/IdleMonitor/Core",
                                        "org.gnome.Mutter.IdleMonitor",
                                        NULL,
                                        &error);
  if (error == NULL)
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_signal), this);

      result = register_active_watch();
      result = result && register_idle_watch();

      if (result)
	{
	  monitor_thread->start();
	}
    }

  if (error != NULL)
    {
      TRACE_MSG("Error: " << error->message);
      g_error_free(error);
      TRACE_EXIT();
      result = false;
    }

  TRACE_EXIT();
  return result;
}

bool
MutterInputMonitor::register_active_watch()
{
  TRACE_ENTER("MutterInputMonitor::register_active_watch");
  GError *error = NULL;
  GVariant *reply = g_dbus_proxy_call_sync(proxy, "AddUserActiveWatch", NULL, G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);

  if (error == NULL)
    {
      g_variant_get(reply, "(u)", &watch_active);
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
MutterInputMonitor::unregister_active_watch()
{
  TRACE_ENTER("MutterInputMonitor::unregister_active_watch");
  GError *error = NULL;
  if (watch_active != 0)
    {
      GVariant *result = g_dbus_proxy_call_sync(proxy, "RemoveWatch", g_variant_new ("(u)", watch_active), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);
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

bool
MutterInputMonitor::register_idle_watch()
{
  TRACE_ENTER("MutterInputMonitor::register_idle_watch");
  GError *error = NULL;
  GVariant *reply = g_dbus_proxy_call_sync(proxy, "AddIdleWatch", g_variant_new("(t)", 500), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);

  if (error == NULL)
    {
      g_variant_get(reply, "(u)", &watch_idle);
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
  if (watch_idle != 0)
    {
      GVariant *result = g_dbus_proxy_call_sync(proxy, "RemoveWatch", g_variant_new("(u)", watch_idle), G_DBUS_CALL_FLAGS_NONE, 10000, NULL, &error);
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
MutterInputMonitor::on_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  (void) proxy;
  (void) sender_name;

  MutterInputMonitor *self = (MutterInputMonitor *)user_data;

  if (g_strcmp0(signal_name, "WatchFired") == 0)
    {
      guint handlerID;
      g_variant_get(parameters, "(u)", &handlerID);

      if (handlerID == self->watch_active)
        {
          self->unregister_active_watch();
          self->active = true;
        }
      else if (handlerID == self->watch_idle)
        {
          self->register_active_watch();
          self->active = false;
        }
    }
}

void
MutterInputMonitor::run()
{
  TRACE_ENTER("MutterInputMonitor::run");

  g_mutex_lock(&mutex);
  while (!abort)
    {
      if (active)
        {
          /* Notify the activity monitor */
          fire_action();
        }

#if GLIB_CHECK_VERSION(2, 32, 0)
      gint64 end_time = g_get_monotonic_time() + G_TIME_SPAN_SECOND;
      g_cond_wait_until(&cond, &mutex, end_time);
#else
      g_usleep(500000);
#endif
    }
  g_mutex_unlock(&mutex);

  TRACE_EXIT();
}
