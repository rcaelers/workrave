/* Copyright (C) 2011 - 2014 Rob Caelers <robc@krandor.nl>
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include "control.h"
#include "timerbox.h"

#define WORKRAVE_DBUS_NAME "org.workrave.Applet"

#define WORKRAVE_DBUS_APPLET_NAME "org.workrave.Workrave"
#define WORKRAVE_DBUS_APPLET_IFACE "org.workrave.AppletInterface"
#define WORKRAVE_DBUS_APPLET_OBJ "/org/workrave/Workrave/UI"

#define WORKRAVE_DBUS_CONTROL_NAME "org.workrave.Workrave"
#define WORKRAVE_DBUS_CONTROL_IFACE "org.workrave.ControlInterface"
#define WORKRAVE_DBUS_CONTROL_OBJ "/org/workrave/Workrave/UI"

#define WORKRAVE_DBUS_CORE_NAME "org.workrave.Workrave"
#define WORKRAVE_DBUS_CORE_IFACE "org.workrave.CoreInterface"
#define WORKRAVE_DBUS_CORE_OBJ "/org/workrave/Workrave/Core"

enum
{
  MENU_CHANGED,
  ALIVE_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

struct _WorkraveTimerboxControlPrivate
{
  GtkImage *image;

  GCancellable *applet_proxy_cancel;
  GDBusProxy *applet_proxy;

  GCancellable *control_proxy_cancel;
  GDBusProxy *control_proxy;

  GCancellable *core_proxy_cancel;
  GDBusProxy *core_proxy;

  guint owner_id;
  guint watch_id;

  gboolean workrave_running;
  gboolean alive;

  gboolean tray_icon_enabled;
  enum WorkraveTimerboxControlTrayIconMode tray_icon_mode;
  gboolean tray_icon_visible_when_not_running;

  guint timer;
  guint startup_timer;
  guint startup_count;
  guint update_count;

  WorkraveTimerbox *timerbox;
};

typedef struct _TimerData TimerData;
struct _TimerData
{
  char *bar_text;
  int slot;
  int bar_secondary_color;
  int bar_secondary_val;
  int bar_secondary_max;
  int bar_primary_color;
  int bar_primary_val;
  int bar_primary_max;
};

G_DEFINE_TYPE_WITH_PRIVATE(WorkraveTimerboxControl, workrave_timerbox_control, G_TYPE_OBJECT);

static void workrave_timerbox_control_class_init(WorkraveTimerboxControlClass *klass);
static void workrave_timerbox_control_init(WorkraveTimerboxControl *self);
static void workrave_timerbox_control_dispose(GObject *object);
static void workrave_timerbox_control_finalize(GObject *object);

static void workrave_timerbox_control_check(WorkraveTimerboxControl *self);
static void workrave_timerbox_control_create_dbus(WorkraveTimerboxControl *self);

static gboolean on_timer(gpointer user_data);
static void on_dbus_applet_ready(GObject *object, GAsyncResult *res, gpointer user_data);
static void on_dbus_core_ready(GObject *object, GAsyncResult *res, gpointer user_data);
static void on_dbus_control_ready(GObject *object, GAsyncResult *res, gpointer user_data);
static void on_dbus_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
static void on_update_timers(WorkraveTimerboxControl *self, GVariant *parameters);
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
static void on_workrave_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data);
static void on_workrave_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data);

GDBusProxy *
workrave_timerbox_control_get_applet_proxy(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  return priv->applet_proxy;
}

GDBusProxy *
workrave_timerbox_control_get_core_proxy(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  return priv->core_proxy;
}

GDBusProxy *
workrave_timerbox_control_get_control_proxy(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  return priv->control_proxy;
}

WorkraveTimerbox *
workrave_timerbox_control_get_timerbox(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  return priv->timerbox;
}

GtkImage *
workrave_timerbox_control_get_image(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (priv->image == NULL)
    {
      priv->image = GTK_IMAGE(gtk_image_new());

      workrave_timerbox_set_enabled(priv->timerbox, FALSE);
      workrave_timerbox_set_force_icon(priv->timerbox, FALSE);
      workrave_timerbox_update(priv->timerbox, priv->image);

      gtk_widget_show(GTK_WIDGET(priv->image));
    }
  return priv->image;
}

static void
workrave_timerbox_control_update_show_tray_icon(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (priv->alive)
    {
      switch (priv->tray_icon_mode)
        {
        case WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_ALWAYS:
          workrave_timerbox_set_force_icon(priv->timerbox, TRUE);
          break;

        case WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_NEVER:
          workrave_timerbox_set_force_icon(priv->timerbox, FALSE);
          break;

        case WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_FOLLOW:
          workrave_timerbox_set_force_icon(priv->timerbox, priv->tray_icon_enabled);
          break;
        }
    }
  else
    {
      workrave_timerbox_set_force_icon(priv->timerbox, priv->tray_icon_visible_when_not_running);
    }
}

void
workrave_timerbox_control_set_tray_icon_mode(WorkraveTimerboxControl *self, enum WorkraveTimerboxControlTrayIconMode mode)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  priv->tray_icon_mode = mode;
  workrave_timerbox_control_update_show_tray_icon(self);
}

void
workrave_timerbox_control_set_tray_icon_visible_when_not_running(WorkraveTimerboxControl *self, gboolean show)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  priv->tray_icon_visible_when_not_running = show;

  workrave_timerbox_control_update_show_tray_icon(self);

  workrave_timerbox_update(priv->timerbox, priv->image);
  gtk_widget_show(GTK_WIDGET(priv->image));
}

static void
workrave_timerbox_control_class_init(WorkraveTimerboxControlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  signals[MENU_CHANGED] = g_signal_new("menu-changed",
                                       G_TYPE_FROM_CLASS(klass),
                                       G_SIGNAL_RUN_LAST | G_SIGNAL_MUST_COLLECT,
                                       0,
                                       NULL,
                                       NULL,
                                       g_cclosure_marshal_VOID__VARIANT,
                                       G_TYPE_NONE,
                                       1,
                                       G_TYPE_VARIANT);

  signals[ALIVE_CHANGED] = g_signal_new("alive-changed",
                                        G_TYPE_FROM_CLASS(klass),
                                        G_SIGNAL_RUN_LAST | G_SIGNAL_MUST_COLLECT,
                                        0,
                                        NULL,
                                        NULL,
                                        g_cclosure_marshal_VOID__BOOLEAN,
                                        G_TYPE_NONE,
                                        1,
                                        G_TYPE_BOOLEAN);

  object_class->dispose = workrave_timerbox_control_dispose;
  object_class->finalize = workrave_timerbox_control_finalize;
}

static void
workrave_timerbox_control_init(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  priv->image = NULL;
  priv->applet_proxy = NULL;
  priv->applet_proxy_cancel = NULL;
  priv->core_proxy = NULL;
  priv->core_proxy_cancel = NULL;
  priv->control_proxy = NULL;
  priv->control_proxy_cancel = NULL;
  priv->owner_id = 0;
  priv->watch_id = 0;
  priv->workrave_running = FALSE;
  priv->alive = FALSE;
  priv->tray_icon_enabled = FALSE;
  priv->tray_icon_mode = WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_FOLLOW;
  priv->tray_icon_visible_when_not_running = FALSE;
  priv->timer = 0;
  priv->startup_timer = 0;
  priv->startup_count = 0;
  priv->timerbox = NULL;
  priv->update_count = 0;

  priv->timerbox = g_object_new(WORKRAVE_TYPE_TIMERBOX, NULL);

  workrave_timerbox_control_create_dbus(self);

  priv->watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                    "org.workrave.Workrave",
                                    G_BUS_NAME_WATCHER_FLAGS_NONE,
                                    on_workrave_appeared,
                                    on_workrave_vanished,
                                    self,
                                    NULL);
}

static void
workrave_timerbox_control_dispose(GObject *object)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(object);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (priv->watch_id != 0)
    {
      g_bus_unwatch_name(priv->watch_id);
    }

  if (priv->owner_id != 0)
    {
      g_bus_unown_name(priv->owner_id);
    }

  if (priv->timer != 0)
    {
      g_source_remove(priv->timer);
      priv->timer = 0;
    }

  if (priv->image != NULL)
    {
      g_object_unref(priv->image);
      priv->image = NULL;
    }

  G_OBJECT_CLASS(workrave_timerbox_control_parent_class)->dispose(object);
  return;
}

static void
workrave_timerbox_control_finalize(GObject *object)
{
  // WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(object);
  G_OBJECT_CLASS(workrave_timerbox_control_parent_class)->finalize(object);
  return;
}

static void
workrave_timerbox_control_start(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (priv->alive)
    {
      return;
    }

  priv->owner_id =
    g_bus_own_name(G_BUS_TYPE_SESSION, WORKRAVE_DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired, NULL, NULL, self, NULL);

  GError *error = NULL;

  if (error == NULL)
    {
      GVariant *result = g_dbus_proxy_call_sync(
        priv->applet_proxy, "Embed", g_variant_new("(bs)", TRUE, WORKRAVE_DBUS_NAME), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

      if (error != NULL)
        {
          g_warning("Could not request embedding for %s: %s", WORKRAVE_DBUS_APPLET_NAME, error->message);
        }
      else
        {
          if (result != NULL)
            {
              g_variant_unref(result);
            }
        }
    }

  if (error == NULL)
    {
      GVariant *result =
        g_dbus_proxy_call_sync(priv->applet_proxy, "GetTrayIconEnabled", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

      if (error != NULL)
        {
          g_warning("Could not request tray icon enabled for %s: %s", WORKRAVE_DBUS_APPLET_NAME, error->message);
        }
      else
        {
          if (result != NULL)
            {
              g_variant_get(result, "(b)", &priv->tray_icon_enabled);
              g_variant_unref(result);
            }
        }
    }

  if (error == NULL)
    {
      GVariant *result =
        g_dbus_proxy_call_sync(priv->core_proxy, "GetOperationMode", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

      if (error != NULL)
        {
          g_warning("Could not request operation mode for %s: %s", WORKRAVE_DBUS_APPLET_NAME, error->message);
        }
      else
        {
          gchar *mode;
          g_variant_get(result, "(s)", &mode);
          workrave_timerbox_set_operation_mode(priv->timerbox, mode);
          g_variant_unref(result);
        }
    }

  if (error == NULL)
    {
      priv->timer = g_timeout_add_seconds(10, on_timer, self);
      priv->alive = TRUE;
      priv->update_count = 0;
      g_signal_emit(self, signals[ALIVE_CHANGED], 0, TRUE);
    }
  else
    {
      g_error_free(error);
    }
}

static void
workrave_timerbox_control_stop(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  if (priv->alive)
    {
      if (priv->timer != 0)
        {
          g_source_remove(priv->timer);
          priv->timer = 0;
        }

      if (priv->startup_timer != 0)
        {
          g_source_remove(priv->startup_timer);
          priv->startup_timer = 0;
        }

      if (priv->owner_id != 0)
        {
          g_bus_unown_name(priv->owner_id);
          priv->owner_id = 0;
        }

      priv->alive = FALSE;

      workrave_timerbox_control_update_show_tray_icon(self);
      workrave_timerbox_set_enabled(priv->timerbox, FALSE);
      workrave_timerbox_update(priv->timerbox, priv->image);
      g_signal_emit(self, signals[ALIVE_CHANGED], 0, FALSE);
    }
}

static gboolean
on_start_delay(gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  workrave_timerbox_control_start(self);
  priv->startup_count++;

  return !priv->alive && priv->startup_count < 15;
}

static void
workrave_timerbox_control_check(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  if (priv->workrave_running && priv->applet_proxy != NULL && priv->core_proxy != NULL && priv->control_proxy != NULL)
    {
      // The Workrave interface may be started after the service becomes
      // available, so introduce a delay.
      priv->startup_count = 0;
      priv->startup_timer = g_timeout_add_seconds(2, on_start_delay, self);
    }
}

static void
workrave_timerbox_control_create_dbus(WorkraveTimerboxControl *self)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  GSettings *settings = g_settings_new("org.workrave.gui");
  gboolean autostart = g_settings_get_boolean(settings, "autostart");
  g_object_unref(settings);

  GDBusProxyFlags flags = autostart ? 0 : G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START;

  priv->applet_proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           flags,
                           NULL,
                           WORKRAVE_DBUS_APPLET_NAME,
                           WORKRAVE_DBUS_APPLET_OBJ,
                           WORKRAVE_DBUS_APPLET_IFACE,
                           priv->applet_proxy_cancel,
                           on_dbus_applet_ready,
                           self);

  priv->core_proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           flags,
                           NULL,
                           WORKRAVE_DBUS_CORE_NAME,
                           WORKRAVE_DBUS_CORE_OBJ,
                           WORKRAVE_DBUS_CORE_IFACE,
                           priv->core_proxy_cancel,
                           on_dbus_core_ready,
                           self);

  priv->control_proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           flags,
                           NULL,
                           WORKRAVE_DBUS_CONTROL_NAME,
                           WORKRAVE_DBUS_CONTROL_OBJ,
                           WORKRAVE_DBUS_CONTROL_IFACE,
                           priv->control_proxy_cancel,
                           on_dbus_control_ready,
                           self);
}

static gboolean
on_timer(gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (priv->alive && priv->update_count == 0)
    {
      workrave_timerbox_set_enabled(priv->timerbox, FALSE);
      workrave_timerbox_set_force_icon(priv->timerbox, priv->tray_icon_visible_when_not_running);
      workrave_timerbox_update(priv->timerbox, priv->image);
    }
  priv->update_count = 0;

  return priv->alive;
}

static void
on_dbus_applet_ready(GObject *object, GAsyncResult *res, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (priv->applet_proxy_cancel != NULL)
    {
      g_object_unref(priv->applet_proxy_cancel);
      priv->applet_proxy_cancel = NULL;
    }

  if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", WORKRAVE_DBUS_APPLET_NAME, error->message);
      g_error_free(error);
    }
  else
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
      priv->applet_proxy = proxy;
    }

  workrave_timerbox_control_check(self);
}

static void
on_dbus_core_ready(GObject *object, GAsyncResult *res, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (priv->core_proxy_cancel != NULL)
    {
      g_object_unref(priv->core_proxy_cancel);
      priv->core_proxy_cancel = NULL;
    }

  if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", WORKRAVE_DBUS_CORE_NAME, error->message);
      g_error_free(error);
    }
  else
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
      priv->core_proxy = proxy;
    }

  workrave_timerbox_control_check(self);
}

static void
on_dbus_control_ready(GObject *object, GAsyncResult *res, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (priv->control_proxy_cancel != NULL)
    {
      g_object_unref(priv->control_proxy_cancel);
      priv->control_proxy_cancel = NULL;
    }

  if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", WORKRAVE_DBUS_CONTROL_NAME, error->message);
      g_error_free(error);
    }
  else
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
      priv->control_proxy = proxy;
    }

  workrave_timerbox_control_check(self);
}

static void
on_dbus_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (g_strcmp0(signal_name, "TimersUpdated") == 0)
    {
      on_update_timers(self, parameters);
    }

  else if (g_strcmp0(signal_name, "MenuUpdated") == 0)
    {
      g_signal_emit(self, signals[MENU_CHANGED], 0, parameters);
    }

  else if (g_strcmp0(signal_name, "TrayIconUpdated") == 0)
    {
      g_variant_get(parameters, "(b)", &priv->tray_icon_enabled);
      workrave_timerbox_control_update_show_tray_icon(self);
      workrave_timerbox_update(priv->timerbox, priv->image);
    }

  else if (g_strcmp0(signal_name, "OperationModeChanged") == 0)
    {
      gchar *mode;
      g_variant_get(parameters, "(s)", &mode);
      workrave_timerbox_set_operation_mode(priv->timerbox, mode);
      workrave_timerbox_update(priv->timerbox, priv->image);
      g_free(mode);
    }
}

static void
on_update_timers(WorkraveTimerboxControl *self, GVariant *parameters)
{
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);

  if (!priv->alive)
    {
      workrave_timerbox_control_start(self);
    }

  priv->update_count++;

  TimerData td[BREAK_ID_SIZEOF];

  g_variant_get(parameters,
                "((siuuuuuu)(siuuuuuu)(siuuuuuu))",
                &td[BREAK_ID_MICRO_BREAK].bar_text,
                &td[BREAK_ID_MICRO_BREAK].slot,
                &td[BREAK_ID_MICRO_BREAK].bar_secondary_color,
                &td[BREAK_ID_MICRO_BREAK].bar_secondary_val,
                &td[BREAK_ID_MICRO_BREAK].bar_secondary_max,
                &td[BREAK_ID_MICRO_BREAK].bar_primary_color,
                &td[BREAK_ID_MICRO_BREAK].bar_primary_val,
                &td[BREAK_ID_MICRO_BREAK].bar_primary_max,
                &td[BREAK_ID_REST_BREAK].bar_text,
                &td[BREAK_ID_REST_BREAK].slot,
                &td[BREAK_ID_REST_BREAK].bar_secondary_color,
                &td[BREAK_ID_REST_BREAK].bar_secondary_val,
                &td[BREAK_ID_REST_BREAK].bar_secondary_max,
                &td[BREAK_ID_REST_BREAK].bar_primary_color,
                &td[BREAK_ID_REST_BREAK].bar_primary_val,
                &td[BREAK_ID_REST_BREAK].bar_primary_max,
                &td[BREAK_ID_DAILY_LIMIT].bar_text,
                &td[BREAK_ID_DAILY_LIMIT].slot,
                &td[BREAK_ID_DAILY_LIMIT].bar_secondary_color,
                &td[BREAK_ID_DAILY_LIMIT].bar_secondary_val,
                &td[BREAK_ID_DAILY_LIMIT].bar_secondary_max,
                &td[BREAK_ID_DAILY_LIMIT].bar_primary_color,
                &td[BREAK_ID_DAILY_LIMIT].bar_primary_val,
                &td[BREAK_ID_DAILY_LIMIT].bar_primary_max);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      workrave_timerbox_set_slot(priv->timerbox, i, td[i].slot);
    }

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      WorkraveTimebar *timebar = workrave_timerbox_get_time_bar(priv->timerbox, i);
      if (timebar != NULL)
        {
          workrave_timerbox_set_enabled(priv->timerbox, TRUE);
          workrave_timerbox_control_update_show_tray_icon(self);
          workrave_timebar_set_progress(timebar, td[i].bar_primary_val, td[i].bar_primary_max, td[i].bar_primary_color);
          workrave_timebar_set_secondary_progress(
            timebar, td[i].bar_secondary_val, td[i].bar_secondary_max, td[i].bar_secondary_color);
          workrave_timebar_set_text(timebar, td[i].bar_text);
        }
    }

  workrave_timerbox_update(priv->timerbox, priv->image);
}

static void
on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  (void)connection;
  (void)name;
  (void)user_data;
}

static void
on_workrave_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  priv->workrave_running = TRUE;
  workrave_timerbox_control_check(self);
}

static void
on_workrave_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  WorkraveTimerboxControl *self = WORKRAVE_TIMERBOX_CONTROL(user_data);
  WorkraveTimerboxControlPrivate *priv = workrave_timerbox_control_get_instance_private(self);
  priv->workrave_running = FALSE;
  workrave_timerbox_control_stop(self);
}
