// Copyright (C) 2002 - 2011 Rob Caelers & Raymond Penners
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

#include "WorkraveApplet.h"
#include "control.h"
#include "commonui/MenuEnums.hh"
#include "commonui/credits.h"
#include "commonui/nls.h"

#include <panel-applet.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

struct _WorkraveAppletPrivate
{
  GtkActionGroup *action_group;
  WorkraveTimerboxControl *timerbox_control;
  GtkImage *image;
  gboolean alive;
};

G_DEFINE_TYPE_WITH_PRIVATE(WorkraveApplet, workrave_applet, PANEL_TYPE_APPLET);

static void workrave_applet_fill(WorkraveApplet *applet);
static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);

struct Menuitems
{
  enum MenuCommand id;
  gboolean autostart;
  gboolean visible_when_not_running;
  char *action;
  char *dbuscmd;
};

static struct Menuitems menu_data[] = {{MENU_COMMAND_OPEN, TRUE, TRUE, "Open", "OpenMain"},
                                       {MENU_COMMAND_PREFERENCES, FALSE, FALSE, "Preferences", "Preferences"},
                                       {MENU_COMMAND_EXERCISES, FALSE, FALSE, "Exercises", "Exercises"},
                                       {MENU_COMMAND_REST_BREAK, FALSE, FALSE, "Restbreak", "RestBreak"},
                                       {MENU_COMMAND_MODE_NORMAL, FALSE, FALSE, "Normal", NULL},
                                       {MENU_COMMAND_MODE_QUIET, FALSE, FALSE, "Quiet", NULL},
                                       {MENU_COMMAND_MODE_SUSPENDED, FALSE, FALSE, "Suspended", NULL},
                                       {MENU_COMMAND_NETWORK_CONNECT, FALSE, FALSE, "Join", "NetworkConnect"},
                                       {MENU_COMMAND_NETWORK_DISCONNECT, FALSE, FALSE, "Disconnect", "NetworkDisconnect"},
                                       {MENU_COMMAND_NETWORK_LOG, FALSE, FALSE, "ShowLog", "NetworkLog"},
                                       {MENU_COMMAND_NETWORK_RECONNECT, FALSE, FALSE, "Reconnect", "NetworkReconnect"},
                                       {MENU_COMMAND_STATISTICS, FALSE, FALSE, "Statistics", "Statistics"},
                                       {MENU_COMMAND_ABOUT, FALSE, TRUE, "About", NULL},
                                       {MENU_COMMAND_MODE_READING, FALSE, FALSE, "ReadingMode", "ReadingMode"},
                                       {MENU_COMMAND_QUIT, FALSE, FALSE, "Quit", "Quit"}};

int
lookup_menu_index_by_id(enum MenuCommand id)
{
  for (int i = 0; i < sizeof(menu_data) / sizeof(struct Menuitems); i++)
    {
      if (menu_data[i].id == id)
        {
          return i;
        }
    }

  return -1;
}

int
lookup_menu_index_by_action(const char *action)
{
  for (int i = 0; i < sizeof(menu_data) / sizeof(struct Menuitems); i++)
    {
      if (g_strcmp0(menu_data[i].action, action) == 0)
        {
          return i;
        }
    }

  return -1;
}

void
on_alive_changed(gpointer instance, gboolean alive, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  applet->priv->alive    = alive;

  if (!alive)
    {
      for (int i = 0; i < sizeof(menu_data) / sizeof(struct Menuitems); i++)
        {
          GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, menu_data[i].action);
          gtk_action_set_visible(action, menu_data[i].visible_when_not_running);
        }
    }
}

void
on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  GVariantIter *iter;
  g_variant_get(parameters, "(a(sii))", &iter);

  char *text;
  int id;
  int flags;

  gboolean visible[sizeof(menu_data) / sizeof(struct Menuitems)];
  for (int i = 0; i < sizeof(menu_data) / sizeof(struct Menuitems); i++)
    {
      visible[i] = menu_data[i].visible_when_not_running;
    }

  while (g_variant_iter_loop(iter, "(sii)", &text, &id, &flags))
    {
      int index = lookup_menu_index_by_id((enum MenuCommand)id);
      if (index == -1)
        {
          continue;
        }

      GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, menu_data[index].action);

      if (flags & MENU_ITEM_FLAG_SUBMENU_END || flags & MENU_ITEM_FLAG_SUBMENU_BEGIN)
        {
          continue;
        }

      visible[index] = TRUE;

      if (GTK_IS_TOGGLE_ACTION(action))
        {
          GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
          gtk_toggle_action_set_active(toggle, flags & MENU_ITEM_FLAG_ACTIVE);
        }
    }

  g_variant_iter_free(iter);

  for (int i = 0; i < sizeof(menu_data) / sizeof(struct Menuitems); i++)
    {
      GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, menu_data[i].action);
      gtk_action_set_visible(action, visible[i]);
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
on_menu_about(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  GdkPixbuf *pixbuf     = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL);
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

  gtk_container_set_border_width(GTK_CONTAINER(about), 5);

  gtk_show_about_dialog(NULL,
                        "name",
                        "Workrave",
#ifdef WORKRAVE_GIT_VERSION
                        "version",
                        PACKAGE_VERSION "\n(" WORKRAVE_GIT_VERSION ")",
#else
                        "version",
                        PACKAGE_VERSION,
#endif
                        "copyright",
                        workrave_copyright,
                        "website",
                        "http://www.workrave.org",
                        "website_label",
                        "www.workrave.org",
                        "comments",
                        _("This program assists in the prevention and recovery"
                          " of Repetitive Strain Injury (RSI)."),
                        "translator-credits",
                        workrave_translators,
                        "authors",
                        workrave_authors,
                        "logo",
                        pixbuf,
                        NULL);
  g_object_unref(pixbuf);
}

static void
on_menu_command(GtkAction *action, WorkraveApplet *applet)
{
  int index = lookup_menu_index_by_action(gtk_action_get_name(action));
  if (index == -1)
    {
      return;
    }

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        menu_data[index].dbuscmd,
                        NULL,
                        menu_data[index].autostart ? G_DBUS_CALL_FLAGS_NONE : G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback)dbus_call_finish,
                        applet);
    }
}

static void
on_menu_toggle(GtkAction *action, WorkraveApplet *applet)
{
  gboolean new_state = FALSE;

  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      new_state               = gtk_toggle_action_get_active(toggle);
    }

  int index = lookup_menu_index_by_action(gtk_action_get_name(action));
  if (index == -1)
    {
      return;
    }

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        menu_data[index].dbuscmd,
                        g_variant_new("(b)", new_state),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback)dbus_call_finish,
                        &applet);
    }
}

static void
on_menu_mode_changed(GtkRadioAction *action, GtkRadioAction *current, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const char *modes[]    = {"normal", "suspended", "quiet"};
  int mode               = 0;

  if (GTK_IS_RADIO_ACTION(action))
    {
      GtkRadioAction *toggle = GTK_RADIO_ACTION(action);
      mode                   = gtk_radio_action_get_current_value(toggle);
    }

  if (mode >= 0 && mode < G_N_ELEMENTS(modes))
    {
      GDBusProxy *proxy = workrave_timerbox_control_get_core_proxy(applet->priv->timerbox_control);
      if (proxy != NULL)
        {
          g_dbus_proxy_call(proxy,
                            "SetOperationMode",
                            g_variant_new("(s)", modes[mode]),
                            G_DBUS_CALL_FLAGS_NO_AUTO_START,
                            -1,
                            NULL,
                            (GAsyncReadyCallback)dbus_call_finish,
                            &applet);
        }
    }
}

static const GtkActionEntry menu_actions[] = {
  {"Open", GTK_STOCK_OPEN, N_("_Open"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Statistics", NULL, N_("_Statistics"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Preferences", GTK_STOCK_PREFERENCES, N_("_Preferences"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Exercises", NULL, N_("_Exercises"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Restbreak", NULL, N_("_Restbreak"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Mode", NULL, N_("_Mode"), NULL, NULL, NULL},

  {"Network", NULL, N_("_Network"), NULL, NULL, NULL},

  {"Join", NULL, N_("_Join"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Disconnect", NULL, N_("_Disconnect"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"Reconnect", NULL, N_("_Reconnect"), NULL, NULL, G_CALLBACK(on_menu_command)},

  {"About", GTK_STOCK_ABOUT, N_("_About"), NULL, NULL, G_CALLBACK(on_menu_about)},

  {"Quit", GTK_STOCK_QUIT, N_("_Quit"), NULL, NULL, G_CALLBACK(on_menu_command)},
};

static const GtkToggleActionEntry toggle_actions[] = {
  {"ShowLog", NULL, N_("Show log"), NULL, NULL, G_CALLBACK(on_menu_toggle), FALSE},

  {"ReadingMode", NULL, N_("Reading mode"), NULL, NULL, G_CALLBACK(on_menu_toggle), FALSE},
};

static const GtkRadioActionEntry mode_actions[] = {
  {"Normal", NULL, N_("Normal"), NULL, NULL, 0},

  {"Suspended", NULL, N_("Suspended"), NULL, NULL, 1},

  {"Quiet", NULL, N_("Quiet"), NULL, NULL, 2},
};

static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, WorkraveApplet *applet)
{
  gboolean ret = FALSE;

  if (event->button == 1)
    {
      GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->priv->timerbox_control);
      if (proxy != NULL)
        {
          g_dbus_proxy_call(proxy,
                            "ButtonClicked",
                            g_variant_new("(u)", event->button),
                            G_DBUS_CALL_FLAGS_NO_AUTO_START,
                            -1,
                            NULL,
                            (GAsyncReadyCallback)dbus_call_finish,
                            &applet);
          ret = TRUE;
        }
    }

  return ret;
}

static void
workrave_applet_class_init(WorkraveAppletClass *class)
{
  g_type_class_add_private(class, sizeof(WorkraveAppletPrivate));
}

static void
workrave_applet_fill(WorkraveApplet *applet)
{
  applet->priv->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->priv->image            = workrave_timerbox_control_get_image(applet->priv->timerbox_control);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "menu-changed", G_CALLBACK(on_menu_changed), applet);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "alive-changed", G_CALLBACK(on_alive_changed), applet);

  workrave_timerbox_control_set_tray_icon_visible_when_not_running(applet->priv->timerbox_control, TRUE);
  workrave_timerbox_control_set_tray_icon_mode(applet->priv->timerbox_control, WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_ALWAYS);

  applet->priv->action_group = gtk_action_group_new("WorkraveAppletActions");
  gtk_action_group_set_translation_domain(applet->priv->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions(applet->priv->action_group, menu_actions, G_N_ELEMENTS(menu_actions), applet);
  gtk_action_group_add_toggle_actions(applet->priv->action_group, toggle_actions, G_N_ELEMENTS(toggle_actions), applet);
  gtk_action_group_add_radio_actions(
    applet->priv->action_group, mode_actions, G_N_ELEMENTS(mode_actions), 0, G_CALLBACK(on_menu_mode_changed), applet);

  gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-gnome-applet-menu.xml", NULL);
  panel_applet_setup_menu_from_file(PANEL_APPLET(applet), ui_path, applet->priv->action_group);
  g_free(ui_path);

  panel_applet_set_flags(PANEL_APPLET(applet), PANEL_APPLET_EXPAND_MINOR);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);
  panel_applet_set_background_widget(PANEL_APPLET(applet), GTK_WIDGET(applet));

  gtk_widget_set_events(GTK_WIDGET(applet), gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed), applet);

  gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(applet->priv->image));

  gtk_widget_show(GTK_WIDGET(applet->priv->image));
  gtk_widget_show(GTK_WIDGET(applet));

  on_alive_changed(NULL, FALSE, applet);
}

static void
workrave_applet_init(WorkraveApplet *applet)
{
  applet->priv = G_TYPE_INSTANCE_GET_PRIVATE(applet, WORKRAVE_TYPE_APPLET, WorkraveAppletPrivate);

  WorkraveAppletPrivate *priv = applet->priv;

  priv->action_group     = NULL;
  priv->image            = NULL;
  priv->timerbox_control = NULL;
  priv->alive            = FALSE;

  workrave_applet_fill(applet);
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

PANEL_APPLET_OUT_PROCESS_FACTORY("WorkraveAppletFactory", WORKRAVE_TYPE_APPLET, applet_factory, NULL)
