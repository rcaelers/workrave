/*
 * Copyright (C) 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include <locale.h>
#include <langinfo.h>
#include <string.h>
#include <time.h>

/* GStuff */
#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>

/* Indicator Stuff */
#if defined(HAVE_INDICATOR_AYATANA)
#  include <libayatana-indicator/indicator.h>
#  include <libayatana-indicator/indicator-object.h>
#  include <libayatana-indicator/indicator-service-manager.h>
#else
#  include <libindicator/indicator.h>
#  include <libindicator/indicator-object.h>
#  include <libindicator/indicator-service-manager.h>
#endif

/* DBusMenu */
#include <libdbusmenu-gtk/menu.h>
#include <libdbusmenu-gtk/menuitem.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "indicator-applet.h"

#include "timerbox.h"
#include "timebar.h"

#define INDICATOR_WORKRAVE_TYPE (indicator_workrave_get_type())
#define INDICATOR_WORKRAVE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), INDICATOR_WORKRAVE_TYPE, IndicatorWorkrave))
#define INDICATOR_WORKRAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), INDICATOR_WORKRAVE_TYPE, IndicatorWorkraveClass))
#define IS_INDICATOR_WORKRAVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), INDICATOR_WORKRAVE_TYPE))
#define IS_INDICATOR_WORKRAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), INDICATOR_WORKRAVE_TYPE))
#define INDICATOR_WORKRAVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), INDICATOR_WORKRAVE_TYPE, IndicatorWorkraveClass))

#define DBUS_NAME ("org.workrave.IndicatorApplet")

typedef struct _IndicatorWorkrave IndicatorWorkrave;
typedef struct _IndicatorWorkraveClass IndicatorWorkraveClass;
typedef struct _IndicatorWorkravePrivate IndicatorWorkravePrivate;

struct _IndicatorWorkraveClass
{
  IndicatorObjectClass parent_class;
};

struct _IndicatorWorkrave
{
  IndicatorObject parent;
};

struct _IndicatorWorkravePrivate
{
  GtkLabel *label;
  GtkImage *image;

  DbusmenuGtkMenu *menu;

  GCancellable *workrave_ui_proxy_cancel;
  GDBusProxy *workrave_ui_proxy;

  GCancellable *workrave_core_proxy_cancel;
  GDBusProxy *workrave_core_proxy;

  guint owner_id;
  guint watch_id;

  gboolean workrave_running;
  gboolean alive;
  gboolean force_icon;
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

typedef struct _MenuItemData MenuItemData;
struct _MenuItemData
{
  GtkWidget *icon;
  GtkWidget *label;
};

GType indicator_workrave_get_type(void);

static void indicator_workrave_class_init(IndicatorWorkraveClass *klass);
static void indicator_workrave_init(IndicatorWorkrave *self);
static void indicator_workrave_dispose(GObject *object);
static void indicator_workrave_finalize(GObject *object);

static GtkImage *get_icon(IndicatorObject *io);
static GtkMenu *get_menu(IndicatorObject *io);
static const gchar *get_accessible_desc(IndicatorObject *io);

static void indicator_workrave_check(IndicatorWorkrave *self);
static void indicator_workrave_create_dbus(IndicatorWorkrave *self);

static gboolean on_timer(gpointer user_data);
static void on_dbus_ui_ready(GObject *object, GAsyncResult *res, gpointer user_data);
static void on_dbus_core_ready(GObject *object, GAsyncResult *res, gpointer user_data);
static void on_dbus_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data);
static void on_update_indicator(IndicatorWorkrave *self, GVariant *parameters);
static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data);
static void on_workrave_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data);
static void on_workrave_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data);

/* Indicator Module Config */
INDICATOR_SET_VERSION
INDICATOR_SET_TYPE(INDICATOR_WORKRAVE_TYPE)

G_DEFINE_TYPE_WITH_CODE(IndicatorWorkrave, indicator_workrave, INDICATOR_OBJECT_TYPE, G_ADD_PRIVATE(IndicatorWorkrave));

static void
indicator_workrave_class_init(IndicatorWorkraveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->dispose = indicator_workrave_dispose;
  object_class->finalize = indicator_workrave_finalize;

  IndicatorObjectClass *io_class = INDICATOR_OBJECT_CLASS(klass);

  io_class->get_menu = get_menu;
  io_class->get_image = get_icon;
  io_class->get_accessible_desc = get_accessible_desc;
}

static void
indicator_workrave_init(IndicatorWorkrave *self)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  priv->label = NULL;
  priv->image = NULL;
  priv->menu = NULL;
  priv->workrave_ui_proxy = NULL;
  priv->workrave_ui_proxy_cancel = NULL;
  priv->workrave_core_proxy = NULL;
  priv->workrave_core_proxy_cancel = NULL;
  priv->owner_id = 0;
  priv->watch_id = 0;
  priv->workrave_running = FALSE;
  priv->alive = FALSE;
  priv->force_icon = FALSE;
  priv->timer = 0;
  priv->startup_timer = 0;
  priv->startup_count = 0;
  priv->timerbox = NULL;
  priv->update_count = 0;

  priv->menu = dbusmenu_gtkmenu_new(WORKRAVE_INDICATOR_MENU_NAME, WORKRAVE_INDICATOR_MENU_OBJ);
  priv->timerbox = g_object_new(WORKRAVE_TYPE_TIMERBOX, NULL);

  indicator_workrave_create_dbus(self);

  priv->watch_id = g_bus_watch_name(G_BUS_TYPE_SESSION,
                                    "org.workrave.Workrave",
                                    G_BUS_NAME_WATCHER_FLAGS_NONE,
                                    on_workrave_appeared,
                                    on_workrave_vanished,
                                    self,
                                    NULL);
}

static void
indicator_workrave_dispose(GObject *object)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(object);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

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

  if (priv->label != NULL)
    {
      g_object_unref(priv->label);
      priv->label = NULL;
    }

  if (priv->image != NULL)
    {
      g_object_unref(priv->image);
      priv->image = NULL;
    }

  if (priv->menu != NULL)
    {
      g_object_unref(G_OBJECT(priv->menu));
      priv->menu = NULL;
    }

  G_OBJECT_CLASS(indicator_workrave_parent_class)->dispose(object);
  return;
}

static void
indicator_workrave_finalize(GObject *object)
{
  // IndicatorWorkrave *self = INDICATOR_WORKRAVE(object);
  G_OBJECT_CLASS(indicator_workrave_parent_class)->finalize(object);
  return;
}

static GtkImage *
get_icon(IndicatorObject *io)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

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

static GtkMenu *
get_menu(IndicatorObject *io)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  return GTK_MENU(priv->menu);
}

static const gchar *
get_accessible_desc(IndicatorObject *io)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(io);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  const gchar *name = NULL;

  if (priv->label != NULL)
    {
      name = "Workrave";
    }
  return name;
}

static void
indicator_workrave_start(IndicatorWorkrave *self)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  if (priv->alive)
    {
      return;
    }

  priv->owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                  DBUS_NAME,
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  on_bus_acquired,
                                  NULL,
                                  NULL,
                                  self,
                                  NULL);

  GError *error = NULL;

  if (error == NULL)
    {
      GVariant *result = g_dbus_proxy_call_sync(priv->workrave_ui_proxy,
                                                "Embed",
                                                g_variant_new("(bs)", TRUE, DBUS_NAME),
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);

      if (error != NULL)
        {
          g_warning("Could not request embedding for %s: %s", WORKRAVE_INDICATOR_SERVICE_NAME, error->message);
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
      GVariant *result = g_dbus_proxy_call_sync(priv->workrave_ui_proxy,
                                                "GetTrayIconEnabled",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);

      if (error != NULL)
        {
          g_warning("Could not request tray icon enabled for %s: %s", WORKRAVE_INDICATOR_SERVICE_NAME, error->message);
        }
      else
        {
          if (result != NULL)
            {
              g_variant_get(result, "(b)", &priv->force_icon);
              g_variant_unref(result);
            }
        }
    }

  if (error == NULL)
    {
      GVariant *result = g_dbus_proxy_call_sync(priv->workrave_core_proxy,
                                                "GetOperationMode",
                                                NULL,
                                                G_DBUS_CALL_FLAGS_NONE,
                                                -1,
                                                NULL,
                                                &error);

      if (error != NULL)
        {
          g_warning("Could not request operation mode for %s: %s", WORKRAVE_INDICATOR_SERVICE_NAME, error->message);
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
    }
  else
    {
      g_error_free(error);
    }
}

static void
indicator_workrave_stop(IndicatorWorkrave *self)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
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

      workrave_timerbox_set_enabled(priv->timerbox, FALSE);
      workrave_timerbox_set_force_icon(priv->timerbox, FALSE);
      workrave_timerbox_update(priv->timerbox, priv->image);
      priv->alive = FALSE;
    }
}

static gboolean
on_start_delay(gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  indicator_workrave_start(self);
  priv->startup_count++;

  return !priv->alive && priv->startup_count < 15;
}

static void
indicator_workrave_check(IndicatorWorkrave *self)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  if (priv->workrave_running && priv->workrave_ui_proxy != NULL && priv->workrave_core_proxy != NULL)
    {
      // The Workrave interface may be started after the service becomes
      // available, so introduce a delay.
      priv->startup_count = 0;
      priv->startup_timer = g_timeout_add_seconds(2, on_start_delay, self);
    }
}

static void
indicator_workrave_create_dbus(IndicatorWorkrave *self)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  GSettings *settings = g_settings_new("org.workrave.gui");
  gboolean autostart = g_settings_get_boolean(settings, "autostart");
  g_object_unref(settings);

  GDBusProxyFlags flags = autostart ?: G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START;

  priv->workrave_ui_proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           flags,
                           NULL,
                           WORKRAVE_INDICATOR_SERVICE_NAME,
                           WORKRAVE_INDICATOR_SERVICE_OBJ,
                           WORKRAVE_INDICATOR_SERVICE_IFACE,
                           priv->workrave_ui_proxy_cancel,
                           on_dbus_ui_ready,
                           self);

  priv->workrave_core_proxy_cancel = g_cancellable_new();
  g_dbus_proxy_new_for_bus(G_BUS_TYPE_SESSION,
                           flags,
                           NULL,
                           WORKRAVE_INDICATOR_CORE_NAME,
                           WORKRAVE_INDICATOR_CORE_OBJ,
                           WORKRAVE_INDICATOR_CORE_IFACE,
                           priv->workrave_core_proxy_cancel,
                           on_dbus_core_ready,
                           self);
}

static gboolean
on_timer(gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  if (priv->alive && priv->update_count == 0)
    {
      workrave_timerbox_set_enabled(priv->timerbox, FALSE);
      workrave_timerbox_set_force_icon(priv->timerbox, FALSE);
      workrave_timerbox_update(priv->timerbox, priv->image);
    }
  priv->update_count = 0;

  return priv->alive;
}

static void
on_dbus_ui_ready(GObject *object, GAsyncResult *res, gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (priv->workrave_ui_proxy_cancel != NULL)
    {
      g_object_unref(priv->workrave_ui_proxy_cancel);
      priv->workrave_ui_proxy_cancel = NULL;
    }

  if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", WORKRAVE_INDICATOR_SERVICE_NAME, error->message);
      g_error_free(error);
    }
  else
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
      priv->workrave_ui_proxy = proxy;
    }

  indicator_workrave_check(self);
}

static void
on_dbus_core_ready(GObject *object, GAsyncResult *res, gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  GError *error = NULL;
  GDBusProxy *proxy = g_dbus_proxy_new_for_bus_finish(res, &error);

  if (priv->workrave_core_proxy_cancel != NULL)
    {
      g_object_unref(priv->workrave_core_proxy_cancel);
      priv->workrave_core_proxy_cancel = NULL;
    }

  if (error != NULL)
    {
      g_warning("Could not grab DBus proxy for %s: %s", WORKRAVE_INDICATOR_CORE_NAME, error->message);
      g_error_free(error);
    }
  else
    {
      g_signal_connect(proxy, "g-signal", G_CALLBACK(on_dbus_signal), self);
      priv->workrave_core_proxy = proxy;
    }

  indicator_workrave_check(self);
}

static void
on_dbus_signal(GDBusProxy *proxy, gchar *sender_name, gchar *signal_name, GVariant *parameters, gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  if (g_strcmp0(signal_name, "TimersUpdated") == 0)
    {
      on_update_indicator(self, parameters);
    }

  else if (g_strcmp0(signal_name, "TrayIconUpdated") == 0)
    {
      g_variant_get(parameters, "(b)", &priv->force_icon);
      workrave_timerbox_set_force_icon(priv->timerbox, priv->force_icon);
      workrave_timerbox_update(priv->timerbox, priv->image);
    }

  else if (g_strcmp0(signal_name, "OperationModeChanged") == 0)
    {
      gchar *mode;
      g_variant_get(parameters, "(s)", &mode);
      workrave_timerbox_set_operation_mode(priv->timerbox, mode);
      workrave_timerbox_update(priv->timerbox, priv->image);
    }
}

static void
on_update_indicator(IndicatorWorkrave *self, GVariant *parameters)
{
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);

  if (!priv->alive)
    {
      indicator_workrave_start(self);
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
          workrave_timerbox_set_force_icon(priv->timerbox, priv->force_icon);
          workrave_timebar_set_progress(timebar, td[i].bar_primary_val, td[i].bar_primary_max, td[i].bar_primary_color);
          workrave_timebar_set_secondary_progress(timebar,
                                                  td[i].bar_secondary_val,
                                                  td[i].bar_secondary_max,
                                                  td[i].bar_secondary_color);
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
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  priv->workrave_running = TRUE;
  indicator_workrave_check(self);
}

static void
on_workrave_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  IndicatorWorkrave *self = INDICATOR_WORKRAVE(user_data);
  IndicatorWorkravePrivate *priv = indicator_workrave_get_instance_private(self);
  priv->workrave_running = FALSE;
  indicator_workrave_stop(self);
}
