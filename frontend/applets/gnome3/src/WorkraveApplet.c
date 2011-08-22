// WorkraveApplet.cc
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
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

#include "WorkraveApplet.h"

#include "credits.h"

#include <panel-applet.h>
#include <gio/gio.h>

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "workrave-gnome-applet-generated.h"

#include "nls.h"

struct _WorkraveAppletPrivate
{
  GtkActionGroup *action_group;
  GtkWidget *hbox;
  GtkWidget *image;
  GtkWidget *socket;
  gboolean has_alpha;

  int size;
  int orientation;
  gboolean last_showlog_state;
  gboolean last_reading_mode_state;
  int last_mode;

  GDBusObjectManagerServer *manager;
  guint service_id;
  guint watcher_id;
  gboolean running;
  GDBusProxy *support;
  GDBusProxy *control;
  GDBusProxy *core;
};

G_DEFINE_TYPE (WorkraveApplet, workrave_applet, PANEL_TYPE_APPLET);

static void workrave_applet_set_all_visible(WorkraveApplet *applet, gboolean visible);
static void workrave_applet_set_visible(WorkraveApplet *applet, gchar *name, gboolean visible);
static void workrave_applet_fill(WorkraveApplet *applet);
static void init_dbus_server(GDBusConnection *connection, WorkraveApplet *applet);
static void init_dbus_client(GDBusConnection *connection, WorkraveApplet *applet);
static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);

/************************************************************************/
/* EXTERNAL DBUS API                                                    */
/************************************************************************/

static gboolean
on_get_socket_id(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  int id = gtk_socket_get_id(GTK_SOCKET(applet->priv->socket));
  g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", id));
  return TRUE;
}

static gboolean
on_get_size(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", applet->priv->size));
  return TRUE;
}

static gboolean
on_get_orientation(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", applet->priv->orientation));
  return TRUE;
}

static gboolean
on_set_menu_status(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation,
                   const gchar *name, gboolean status, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  if (g_str_has_prefix(name, "/commands/"))
    {
      name += 10; // Skip gnome2 prefix for compatibility
    }

  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      gtk_toggle_action_set_active(toggle, status);

    }
  return TRUE;
}

static gboolean
on_get_menu_status(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  if (g_str_has_prefix(name, "/commands/"))
    {
      name += 10; // Skip gnome2 prefix for compatibility
    }

  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      int status = gtk_toggle_action_get_active(toggle);

      g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", status));
    }
  return TRUE;
}

static gboolean
on_set_menu_active(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation,
                   const gchar *name, gboolean status, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  gtk_action_set_visible(action, status);
  return TRUE;
}

static gboolean
on_get_menu_active(WorkraveGnomeAppletInterface *applet_dbus, GDBusMethodInvocation *invocation, const gchar *name, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  int active = gtk_action_get_visible(action);
  g_dbus_method_invocation_return_value(invocation, g_variant_new ("(u)", active));
  return TRUE;
}


/************************************************************************/
/* DBUS                                                                 */
/************************************************************************/

static void
on_bus_name_appeared(GDBusConnection *connection, const gchar *name, const gchar *name_owner, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  applet->priv->running = TRUE;
}

static void
on_bus_name_vanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  applet->priv->running = FALSE;
}
  
static void
on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

static void
on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

static void
on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  init_dbus_server(connection, applet);
  init_dbus_client(connection, applet);
}

static void
on_control_proxy_ready(GObject *source, GAsyncResult *result, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  GError *error = NULL;

  applet->priv->control = g_dbus_proxy_new_for_bus_finish(result, &error);
  if (error != NULL)
    {
      g_warning("Failed to obtain DBUS proxy to UI control: %s", error ? error->message : "");
      g_error_free(error);
    }

}

static void
on_support_proxy_ready(GObject *source, GAsyncResult *result, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  GError *error = NULL;

  applet->priv->support = g_dbus_proxy_new_for_bus_finish(result, &error);
  if (error != NULL)
    {
      g_warning("Failed to obtain DBUS proxy to applet support: %s", error ? error->message : "");
      g_error_free(error);
      return;
    }
  
  if (applet->priv->support != NULL)
    {
      g_dbus_proxy_call(applet->priv->support,
                        "EmbedRequest",
                        NULL,
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        &applet);
    }
}

static void
on_core_proxy_ready(GObject *source, GAsyncResult *result, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  GError *error = NULL;

  applet->priv->core = g_dbus_proxy_new_for_bus_finish(result, &error);
  if (error != NULL)
    {
      g_warning("Failed to obtain DBUS proxy to core: %s", error ? error->message : "");
      g_error_free(error);
    }

}

static void
dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data)
{
  GError *error = NULL;
  GVariant *result;

  result = g_dbus_proxy_call_finish(proxy, res, &error);
  if (error != NULL)
    {
      g_warning("DBUS Failed: %s", error ? error->message : "");
      g_error_free(error);
    }

  if (result != NULL)
    {
      g_variant_unref(result);
    }
}

static void
init_dbus_server(GDBusConnection *connection, WorkraveApplet *applet)
{
  applet->priv->manager = g_dbus_object_manager_server_new("/org/workrave/Workrave/");

  WorkraveGnomeAppletInterface *applet_dbus = workrave_gnome_applet_interface_skeleton_new();
  WorkraveObjectSkeleton *object = workrave_object_skeleton_new("GnomeApplet");

  workrave_object_skeleton_set_gnome_applet_interface(object, applet_dbus);

  g_object_unref(applet);

  g_signal_connect(applet_dbus, "handle-get-socket-id",   G_CALLBACK (on_get_socket_id), applet);
  g_signal_connect(applet_dbus, "handle-get-size",        G_CALLBACK (on_get_size), applet);
  g_signal_connect(applet_dbus, "handle-get-orientation", G_CALLBACK (on_get_orientation), applet);
  g_signal_connect(applet_dbus, "handle-get-menu-status", G_CALLBACK (on_get_menu_status), applet);
  g_signal_connect(applet_dbus, "handle-set-menu-status", G_CALLBACK (on_set_menu_status), applet);
  g_signal_connect(applet_dbus, "handle-get-menu-active", G_CALLBACK (on_get_menu_active), applet);
  g_signal_connect(applet_dbus, "handle-set-menu-active", G_CALLBACK (on_set_menu_active), applet);

  g_dbus_object_manager_server_export(applet->priv->manager, G_DBUS_OBJECT_SKELETON(object));
  g_object_unref(object);

  g_dbus_object_manager_server_set_connection(applet->priv->manager, connection);
}

static void
init_dbus_client(GDBusConnection *connection, WorkraveApplet *applet)
{
  g_dbus_proxy_new(connection,
                   G_DBUS_PROXY_FLAGS_NONE,
                   NULL,
                   "org.workrave.Workrave.Activator",
                   "/org/workrave/Workrave/UI",
                   "org.workrave.GnomeAppletSupportInterface",
                   NULL, 
                   on_support_proxy_ready,
                   applet);

  g_dbus_proxy_new(connection,
                   G_DBUS_PROXY_FLAGS_NONE,
                   NULL,
                   "org.workrave.Workrave.Activator",
                   "/org/workrave/Workrave/UI",
                   "org.workrave.ControlInterface",
                   NULL, 
                   on_control_proxy_ready,
                   applet);

  g_dbus_proxy_new(connection,
                   G_DBUS_PROXY_FLAGS_NONE,
                   NULL,
                   "org.workrave.Workrave.Activator",
                   "/org/workrave/Workrave/Core",
                   "org.workrave.CoreInterface",
                   NULL, 
                   on_core_proxy_ready,
                   applet);


  applet->priv->watcher_id = g_bus_watch_name_on_connection(connection,
                                                            "org.workrave.Workrave",
                                                            G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                            on_bus_name_appeared,
                                                            on_bus_name_vanished,
                                                            applet,
                                                            NULL);
  
}

static void
workrave_dbus_server_init(WorkraveApplet *applet)
{
  applet->priv->service_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                            DBUS_SERVICE_APPLET,
                                            G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                                            G_BUS_NAME_OWNER_FLAGS_REPLACE,
                                            on_bus_acquired,
                                            on_name_acquired,
                                            on_name_lost,
                                            applet,
                                            NULL);


}


static void
workrave_dbus_server_cleanup(WorkraveApplet *applet)
{
  g_bus_unwatch_name(applet->priv->watcher_id);
  g_bus_unown_name (applet->priv->service_id);
  g_object_unref(applet->priv->control);
  g_object_unref(applet->priv->support);
  g_object_unref(applet->priv->core);
}


static gboolean
workrave_is_running(WorkraveApplet *applet)
{
	return applet->priv->running;
}

/************************************************************************/
/* GNOME::Applet                                                        */
/************************************************************************/


static void
on_menu_about(GtkAction *action, WorkraveApplet *applet)
{
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL);
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

  gtk_container_set_border_width(GTK_CONTAINER(about), 5);

  gtk_show_about_dialog(NULL,
                        "name", "Workrave",
#ifdef GIT_VERSION
                        "version", PACKAGE_VERSION "\n(" GIT_VERSION ")",
#else
                        "version", PACKAGE_VERSION,
#endif
                        "copyright", workrave_copyright,
                        "website", "http://www.workrave.org",
                        "website_label", "www.workrave.org",
                        "comments", _("This program assists in the prevention and recovery"
                                      " of Repetitive Strain Injury (RSI)."),
                        "translator-credits", workrave_translators,
                        "authors", workrave_authors,
                        "logo", pixbuf,
                         NULL);
  g_object_unref(pixbuf);
}


static void
menu_call(WorkraveApplet *applet, char *call)
{
  g_dbus_proxy_call(applet->priv->control,
                    call,
                    NULL,
                    G_DBUS_CALL_FLAGS_NONE,
                    -1,
                    NULL,
                    (GAsyncReadyCallback) dbus_call_finish,
                    applet);
}

static void
on_menu_open(GtkAction *action, WorkraveApplet *applet)
{
  menu_call(applet, "OpenMain");
}

static void
on_menu_preferences(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "Preferences");
}


static void
on_menu_exercises(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "Exercises");
}

static void
on_menu_statistics(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "Statistics");
}

static void
on_menu_restbreak(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "RestBreak");
}



static void
on_menu_connect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "NetworkConnect");
}


static void
on_menu_disconnect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "NetworkDisconnect");
}

static void
on_menu_reconnect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "NetworkReconnect");
}



static void
on_menu_quit(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running(applet))
    {
      return;
    }

  menu_call(applet, "Quit");
}


static gboolean
plug_removed(GtkSocket *socket, WorkraveApplet *applet)
{
  gtk_widget_show(GTK_WIDGET(applet->priv->image));
  gtk_widget_hide(GTK_WIDGET(applet->priv->socket));
  workrave_applet_set_all_visible(applet, FALSE);
  return TRUE;
}


static gboolean
plug_added(GtkSocket *socket, WorkraveApplet *applet)
{
  gtk_widget_hide(GTK_WIDGET(applet->priv->image));
  gtk_widget_show(GTK_WIDGET(applet->priv->socket));
  workrave_applet_set_all_visible(applet, TRUE);

  return TRUE;
}

static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, WorkraveApplet *applet)
{
  gboolean ret = FALSE;

  if (event->button == 1)
    {
      if (applet->priv->support != NULL && workrave_is_running(applet))
        {
          g_dbus_proxy_call(applet->priv->support,
                            "ButtonClicked",
                            g_variant_new("(u)", event->button),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            (GAsyncReadyCallback) dbus_call_finish,
                            &applet);
          ret = TRUE;
        }
    }

  return ret;
}

static void
showlog_callback(GtkAction *action, WorkraveApplet *applet)
{
  gboolean new_state = FALSE;

  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      new_state = gtk_toggle_action_get_active(toggle);
    }

  applet->priv->last_showlog_state = new_state;

  if (applet->priv->control != NULL && workrave_is_running(applet))
    {
      g_dbus_proxy_call(applet->priv->control,
                            "NetworkLog",
                            g_variant_new("(b)", new_state),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            (GAsyncReadyCallback) dbus_call_finish,
                            applet);
    }
}


static void
reading_mode_callback(GtkAction *action, WorkraveApplet *applet)
{
  gboolean new_state = FALSE;

  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      new_state = gtk_toggle_action_get_active(toggle);
    }

  applet->priv->last_reading_mode_state = new_state;

  if (applet->priv->control != NULL && workrave_is_running(applet))
    {
      g_dbus_proxy_call(applet->priv->control,
                            "ReadingMode",
                            g_variant_new("(b)", new_state),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            (GAsyncReadyCallback) dbus_call_finish,
                            &applet);
    }
}

static void
mode_callback(GtkRadioAction *action, GtkRadioAction *current, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const char *modes[] = { "normal", "suspended", "quiet" };
  int mode = 0;

  if (GTK_IS_RADIO_ACTION(action))
    {
      GtkRadioAction *toggle = GTK_RADIO_ACTION(action);
      mode = gtk_radio_action_get_current_value(toggle);
    }

  if (mode >= 0 && mode < G_N_ELEMENTS(modes))
    {
      applet->priv->last_mode = mode;

      if (applet->priv->core != NULL && workrave_is_running(applet))
        {
          g_dbus_proxy_call(applet->priv->core,
                            "SetOperationMode",
                            g_variant_new("(s)", modes[mode]),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            (GAsyncReadyCallback) dbus_call_finish,
                            &applet);
        }
    }
}


static void
workrave_applet_set_visible(WorkraveApplet *applet, gchar *name, gboolean visible)
{
  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  gtk_action_set_visible(action, visible);
}


static void
workrave_applet_set_all_visible(WorkraveApplet *applet, gboolean visible)
{
  workrave_applet_set_visible(applet, "Preferences", visible);
  workrave_applet_set_visible(applet, "Restbreak", visible);
  workrave_applet_set_visible(applet, "Network", visible);
  workrave_applet_set_visible(applet, "Normal", visible);
  workrave_applet_set_visible(applet, "Suspended", visible);
  workrave_applet_set_visible(applet, "Quiet", visible);
  workrave_applet_set_visible(applet, "Mode", visible);
  workrave_applet_set_visible(applet, "Statistics", visible);
  workrave_applet_set_visible(applet, "Exercises", visible);
  workrave_applet_set_visible(applet, "ReadingMode", visible);
  workrave_applet_set_visible(applet, "Quit", visible);
}


static const GtkActionEntry menu_actions [] = {
  { "Open", GTK_STOCK_OPEN, N_("_Open"),
    NULL, NULL,
    G_CALLBACK(on_menu_open) },

  { "Statistics", NULL, N_("_Statistics"),
    NULL, NULL,
    G_CALLBACK(on_menu_statistics) },

  { "Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"),
    NULL, NULL,
    G_CALLBACK(on_menu_preferences) },

  { "Exercises", NULL, N_("_Exercises"),
    NULL, NULL,
    G_CALLBACK(on_menu_exercises) },

  { "Restbreak", NULL, N_("_Restbreak"),
    NULL, NULL,
    G_CALLBACK(on_menu_restbreak) },

  { "Mode", NULL, N_("_Mode"),
    NULL, NULL,
    NULL },

  { "Network", NULL, N_("_Network"),
    NULL, NULL,
    NULL },

  { "Join", NULL, N_("_Join"),
    NULL, NULL,
    G_CALLBACK(on_menu_connect) },

  { "Disconnect", NULL, N_("_Disconnect"),
    NULL, NULL,
    G_CALLBACK(on_menu_disconnect) },

  { "Reconnect", NULL, N_("_Reconnect"),
    NULL, NULL,
    G_CALLBACK(on_menu_reconnect) },

  { "About", GTK_STOCK_ABOUT, N_("_About"),
    NULL, NULL,
    G_CALLBACK(on_menu_about) },

  { "Quit", GTK_STOCK_QUIT, N_("_Quit"),
    NULL, NULL,
    G_CALLBACK(on_menu_quit) },
};


static const GtkToggleActionEntry toggle_actions[] = {
    { "ShowLog", NULL, N_("Show log"),
      NULL, NULL,
      G_CALLBACK(showlog_callback), FALSE },

    { "ReadingMode", NULL, N_("Reading mode"),
      NULL, NULL,
      G_CALLBACK(reading_mode_callback), FALSE },
};


static const GtkRadioActionEntry mode_actions[] = {
    { "Normal", NULL, N_("Normal"),
      NULL, NULL,
      0 },

    { "Suspended", NULL, N_("Suspended"),
      NULL, NULL,
      1 },

    { "Quiet", NULL, N_("Quiet"),
      NULL, NULL,
      2 },
};


static void
workrave_applet_socket_realize(GtkWidget *widget, gpointer user_data)
{
  GtkSocket *socket = GTK_SOCKET(widget);
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  GdkWindow *window = gtk_widget_get_window(widget);

  if (applet->priv->has_alpha)
    {
      cairo_pattern_t *transparent = cairo_pattern_create_rgba(255, 0, 0, 128);
      gdk_window_set_background_pattern(window, transparent);
      gdk_window_set_composited(window, TRUE);
      cairo_pattern_destroy(transparent);

      gtk_widget_set_app_paintable(GTK_WIDGET(socket), TRUE);
      gtk_widget_set_double_buffered(GTK_WIDGET(socket), TRUE);
      gtk_container_set_border_width(GTK_CONTAINER(socket), 0);
    }

  // TODO: background relative to parent.
}

static void
workrave_applet_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  if (applet->priv->has_alpha)
    {
      GtkAllocation allocation;

      gtk_widget_get_allocation(applet->priv->socket, &allocation);

      cairo_save (cr);
      gdk_cairo_set_source_window (cr,
                                   gtk_widget_get_window (applet->priv->socket),
				   allocation.x,
				   allocation.y);
      cairo_rectangle (cr, allocation.x, allocation.y, allocation.width, allocation.height);
      cairo_clip (cr);
      cairo_paint (cr);
      cairo_restore (cr);
    }
}

static void
workrave_applet_realize(GtkWidget *widget)
{
  GTK_WIDGET_CLASS(workrave_applet_parent_class)->realize(widget);
}

static void
workrave_applet_unrealize(GtkWidget *widget)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(widget);
  workrave_dbus_server_cleanup(applet);
  GTK_WIDGET_CLASS(workrave_applet_parent_class)->unrealize (widget);
}


static void
workrave_applet_change_orient(PanelApplet *panel, PanelAppletOrient o)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(panel);
  char *str = "";

  switch (o)
    {
    case PANEL_APPLET_ORIENT_UP:
      applet->priv->orientation = 0;
      str = "up";
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
      applet->priv->orientation = 1;
      str = "right";
      break;
    case PANEL_APPLET_ORIENT_DOWN:
      applet->priv->orientation = 2;
      str = "down";
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      applet->priv->orientation = 3;
      str = "left";
      break;
    }

  if (applet->priv->support != NULL && workrave_is_running(applet))
    {
      g_dbus_proxy_call(applet->priv->support,
                        "SetOrientation",
                        g_variant_new("(s)", str),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        &applet);
    }
}


static void
force_no_focus_padding(GtkWidget *widget)
{
  GtkCssProvider *provider = gtk_css_provider_new();
  gtk_css_provider_load_from_data(provider,
                                  "WorkraveApplet {\n"
                                  " -GtkWidget-focus-line-width: 0px;\n"
                                  " -GtkWidget-focus-padding: 0px;\n"
                                  "}", -1, NULL);

  gtk_style_context_add_provider(gtk_widget_get_style_context(widget),
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
}


static void
workrave_applet_class_init(WorkraveAppletClass *class)
{
  GtkWidgetClass   *widget_class = GTK_WIDGET_CLASS(class);
  PanelAppletClass *applet_class = PANEL_APPLET_CLASS(class);

  widget_class->realize = workrave_applet_realize;
  widget_class->unrealize = workrave_applet_unrealize;

  applet_class->change_orient = workrave_applet_change_orient;

  g_type_class_add_private(class, sizeof(WorkraveAppletPrivate));
}


static void
workrave_applet_fill(WorkraveApplet *applet)
{
  GdkPixbuf *pixbuf = NULL;
  PanelAppletOrient orient;

  applet->priv->action_group = gtk_action_group_new("WorkraveAppletActions");
  gtk_action_group_set_translation_domain(applet->priv->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions(applet->priv->action_group,
                               menu_actions,
                               G_N_ELEMENTS (menu_actions),
                               applet);
  gtk_action_group_add_toggle_actions(applet->priv->action_group,
                                      toggle_actions,
                                      G_N_ELEMENTS (toggle_actions),
                                      applet);
  gtk_action_group_add_radio_actions (applet->priv->action_group,
                                      mode_actions,
                                      G_N_ELEMENTS(mode_actions),
                                      0,
                                      G_CALLBACK(mode_callback),
                                      applet);

  gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-applet-menu.xml", NULL);
  panel_applet_setup_menu_from_file(PANEL_APPLET(applet), ui_path, applet->priv->action_group);
  g_free(ui_path);

  panel_applet_set_flags(PANEL_APPLET(applet), PANEL_APPLET_EXPAND_MINOR);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);
  panel_applet_set_background_widget(PANEL_APPLET(applet), GTK_WIDGET(applet));

  // Socket.
  applet->priv->socket = gtk_socket_new();
  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(applet));
  GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(applet));

  if (gdk_screen_get_rgba_visual(screen) != NULL && gdk_display_supports_composite(display))
    {
      GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
      gtk_widget_set_visual(applet->priv->socket, visual);
      applet->priv->has_alpha = TRUE;
    }
  else
    {
      applet->priv->has_alpha = FALSE;
    }

	g_signal_connect(applet->priv->socket, "realize", G_CALLBACK(workrave_applet_socket_realize), applet);

  // Image
  pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave-icon-medium.png", NULL);
  applet->priv->image = gtk_image_new_from_pixbuf(pixbuf);

  // Container.
  applet->priv->hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_end(GTK_BOX(applet->priv->hbox), applet->priv->socket, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(applet->priv->hbox), applet->priv->image, TRUE, TRUE, 0);
	g_signal_connect(applet->priv->hbox, "draw", G_CALLBACK(workrave_applet_draw), applet);

  gtk_container_set_border_width(GTK_CONTAINER(applet->priv->hbox), 0);

  applet->priv->size = panel_applet_get_size(PANEL_APPLET(applet));

  orient = panel_applet_get_orient(PANEL_APPLET(applet));

  switch (orient)
    {
    case PANEL_APPLET_ORIENT_UP:
      applet->priv->orientation = 0;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
      applet->priv->orientation = 1;
      break;
    case PANEL_APPLET_ORIENT_DOWN:
      applet->priv->orientation = 2;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      applet->priv->orientation = 3;
      break;
    }

  force_no_focus_padding(GTK_WIDGET(applet));
  force_no_focus_padding(GTK_WIDGET(applet->priv->socket));
  force_no_focus_padding(GTK_WIDGET(applet->priv->image));
  force_no_focus_padding(GTK_WIDGET(applet->priv->hbox));

  // Signals.
  g_signal_connect(applet->priv->socket, "plug_removed", G_CALLBACK(plug_removed), applet->priv);
  g_signal_connect(applet->priv->socket, "plug_added", G_CALLBACK(plug_added), applet->priv);

  gtk_widget_set_events(GTK_WIDGET(applet), gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),  applet);

  gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(applet->priv->hbox));

  gtk_widget_show(GTK_WIDGET(applet->priv->image));
  gtk_widget_hide(GTK_WIDGET(applet->priv->socket));
  gtk_widget_show(GTK_WIDGET(applet->priv->hbox));
  gtk_widget_show(GTK_WIDGET(applet));
}


static void
workrave_applet_init(WorkraveApplet *applet)
{
  applet->priv = G_TYPE_INSTANCE_GET_PRIVATE(applet, WORKRAVE_TYPE_APPLET, WorkraveAppletPrivate);

  WorkraveAppletPrivate *priv = applet->priv;

  priv->action_group = NULL;
  priv->hbox = NULL;
  priv->image = NULL;
  priv->socket = NULL;
  priv->has_alpha = FALSE;
  priv->size = 48;
  priv->orientation = 0;
  priv->last_showlog_state = FALSE;
  priv->last_reading_mode_state = FALSE;
  priv->last_mode = 0;
  priv->manager = NULL;
  priv->service_id = 0;
  priv->watcher_id = 0;
  priv->running = FALSE;
  priv->support = NULL;
  priv->control = NULL;
  priv->core = NULL;
  
  workrave_dbus_server_init(applet);
  workrave_applet_fill(applet);

  force_no_focus_padding(GTK_WIDGET(applet));
}


static gboolean
applet_factory(PanelApplet *applet, const gchar *iid, gpointer user_data)
{
  if (g_strcmp0(iid, "WorkraveApplet") == 0)
    {
      gtk_widget_show_all(GTK_WIDGET(applet));
      return TRUE;
    }

  return FALSE;
}


PANEL_APPLET_OUT_PROCESS_FACTORY("WorkraveAppletFactory",
                                 WORKRAVE_TYPE_APPLET,
                                 applet_factory,
                                 NULL)
