// Copyright (C) 2002 - 2014 Rob Caelers & Raymond Penners
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
#include "control.h"
#include "commonui/MenuEnums.hh"
#include "commonui/credits.h"
#include "nls.h"

#include <panel-applet.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

// These two functions are implemented in applets/common/src/control.c,
// but not defined in any header file.
void workrave_timerbox_control_set_tray_icon_mode(WorkraveTimerboxControl *self, enum WorkraveTimerboxControlTrayIconMode mode);
void workrave_timerbox_control_set_tray_icon_visible_when_not_running(WorkraveTimerboxControl *self, gboolean show);

struct _WorkraveAppletPrivate
{
  GSimpleActionGroup *action_group;
  WorkraveTimerboxControl *timerbox_control;
  GtkImage *image;
  gboolean alive;
};

G_DEFINE_TYPE (WorkraveApplet, workrave_applet, PANEL_TYPE_APPLET);

static void workrave_applet_fill(WorkraveApplet *applet);
static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);

struct Menuitems
{
  enum MenuCommand id;
  gboolean autostart;
  gboolean visible_when_not_running;
  char *action;
  char *state;
  char *dbuscmd;
};

static struct Menuitems menu_data[] =
  {
    { MENU_COMMAND_OPEN,                  TRUE,  TRUE,  "open",         NULL,         "OpenMain"          },
    { MENU_COMMAND_PREFERENCES,           FALSE, FALSE, "preferences",  NULL,         "Preferences"       },
    { MENU_COMMAND_EXERCISES,             FALSE, FALSE, "exercises",    NULL,         "Exercises"         },
    { MENU_COMMAND_REST_BREAK,            FALSE, FALSE, "restbreak",    NULL,         "RestBreak"         },
    { MENU_COMMAND_MODE_NORMAL,           FALSE, FALSE, "mode",         "normal",     NULL                },
    { MENU_COMMAND_MODE_QUIET,            FALSE, FALSE, "mode",         "quiet",      NULL                },
    { MENU_COMMAND_MODE_SUSPENDED,        FALSE, FALSE, "mode",         "suspended",  NULL                },
    { MENU_COMMAND_NETWORK_CONNECT,       FALSE, FALSE, "join",         NULL,         "NetworkConnect"    },
    { MENU_COMMAND_NETWORK_DISCONNECT,    FALSE, FALSE, "disconnect",   NULL,         "NetworkDisconnect" },
    { MENU_COMMAND_NETWORK_LOG,           FALSE, FALSE, "showlog",      NULL,         "NetworkLog"        },
    { MENU_COMMAND_NETWORK_RECONNECT,     FALSE, FALSE, "reconnect",    NULL,         "NetworkReconnect"  },
    { MENU_COMMAND_STATISTICS,            FALSE, FALSE, "statistics",   NULL,         "Statistics"        },
    { MENU_COMMAND_ABOUT,                 FALSE, TRUE,  "about",        NULL,         NULL                },
    { MENU_COMMAND_MODE_READING,          FALSE, FALSE, "readingmode",  NULL,         "ReadingMode"       },
    { MENU_COMMAND_QUIT,                  FALSE, FALSE, "quit",         NULL,         "Quit"              }
  };

//    { 0,                                  FALSE, FALSE, "network",      NULL,         NULL                },

int
lookup_menu_index_by_id(enum MenuCommand id)
{
  for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
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
  for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
    {
      if (g_strcmp0(menu_data[i].action, action) == 0)
        {
          return i;
        }
    }

  return -1;
}

void on_alive_changed(gpointer instance, gboolean alive, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  applet->priv->alive = alive;

  if (!alive)
    {
      for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
        {
          GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), menu_data[i].action);
          g_simple_action_set_enabled(G_SIMPLE_ACTION(action), menu_data[i].visible_when_not_running);
        }
    }
}

void on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  GVariantIter *iter;
  g_variant_get (parameters, "(a(sii))", &iter);

  char *text;
  int id;
  int flags;

  gboolean visible[sizeof(menu_data)/sizeof(struct Menuitems)];
  for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
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

      GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), menu_data[index].action);

      if (flags & MENU_ITEM_FLAG_SUBMENU_END ||
          flags & MENU_ITEM_FLAG_SUBMENU_BEGIN)
        {
          continue;
        }

      visible[index] = TRUE;

      if (g_action_get_state_type(G_ACTION(action)) != NULL)
        {
          if (menu_data[index].state == NULL)
            {
              g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_boolean(flags & MENU_ITEM_FLAG_ACTIVE));
            }
          else
            {
              if (flags & MENU_ITEM_FLAG_ACTIVE)
                {
                  g_simple_action_set_state(G_SIMPLE_ACTION(action), g_variant_new_string(menu_data[index].state));
                }
            }
        }
    }

  g_variant_iter_free (iter);

  for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
    {
      GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), menu_data[i].action);
      g_simple_action_set_enabled(G_SIMPLE_ACTION(action), visible[i]);
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
on_menu_command(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  int index = lookup_menu_index_by_action(g_action_get_name(G_ACTION(action)));
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
                        (GAsyncReadyCallback) dbus_call_finish,
                        applet);
    }
}

static void
on_menu_mode(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  g_action_change_state (G_ACTION(action), parameter);
}

static void
on_menu_toggle(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  GVariant *state = g_action_get_state(G_ACTION(action));
  gboolean new_state = !g_variant_get_boolean(state);
  g_action_change_state (G_ACTION(action), g_variant_new_boolean(new_state));
  g_variant_unref(state);
}

static void
on_menu_toggle_changed(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  gboolean new_state = g_variant_get_boolean(value);
  int index = lookup_menu_index_by_action(g_action_get_name(G_ACTION(action)));
  if (index == -1)
    {
      return;
    }

  g_simple_action_set_state(action, value);

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        menu_data[index].dbuscmd,
                        g_variant_new("(b)", new_state),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        &applet);
    }
}

static void
on_menu_mode_changed(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const gchar *mode = g_variant_get_string(value, 0);

  g_simple_action_set_state(action, value);

  GDBusProxy *proxy = workrave_timerbox_control_get_core_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "SetOperationMode",
                        g_variant_new("(s)", mode),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        &applet);
    }
}

static const GActionEntry menu_actions [] = {
  { "open",        on_menu_command    },
  { "statistics",  on_menu_command    },
  { "preferences", on_menu_command    },
  { "exercises",   on_menu_command    },
  { "restbreak",   on_menu_command    },
  { "mode",        on_menu_mode,   "s", "'normal'", on_menu_mode_changed },
  { "join",        on_menu_command    },
  { "disconnect",  on_menu_command    },
  { "reconnect",   on_menu_command    },
  { "showlog",     on_menu_toggle, NULL, "false", on_menu_toggle_changed },
  { "readingmode", on_menu_toggle, NULL, "false", on_menu_toggle_changed },
  { "about",       on_menu_about      },
  { "quit",        on_menu_command    },
};

static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, WorkraveApplet *applet)
{
  gboolean ret = FALSE;

  if (event->button == 1)
    {
      GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->priv->timerbox_control);
      if (proxy != NULL)
        {          g_dbus_proxy_call(proxy,
                            "ButtonClicked",
                            g_variant_new("(u)", event->button),
                            G_DBUS_CALL_FLAGS_NO_AUTO_START,
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
workrave_applet_class_init(WorkraveAppletClass *class)
{
  g_type_class_add_private(class, sizeof(WorkraveAppletPrivate));
}

static void
workrave_applet_fill(WorkraveApplet *applet)
{
  applet->priv->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->priv->image = workrave_timerbox_control_get_image(applet->priv->timerbox_control);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "menu-changed", G_CALLBACK(on_menu_changed),  applet);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "alive-changed", G_CALLBACK(on_alive_changed),  applet);

  workrave_timerbox_control_set_tray_icon_visible_when_not_running(applet->priv->timerbox_control, TRUE);
  workrave_timerbox_control_set_tray_icon_mode(applet->priv->timerbox_control, WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_ALWAYS);

  applet->priv->action_group = g_simple_action_group_new();
  g_action_map_add_action_entries (G_ACTION_MAP (applet->priv->action_group),
                                   menu_actions,
                                   G_N_ELEMENTS (menu_actions),
                                   applet);

  gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-gnome-applet-menu.xml", NULL);
  panel_applet_setup_menu_from_file(PANEL_APPLET(applet), ui_path, applet->priv->action_group, GETTEXT_PACKAGE);
  g_free(ui_path);

  gtk_widget_insert_action_group (GTK_WIDGET (applet), "workrave",
                                  G_ACTION_GROUP (applet->priv->action_group));

  panel_applet_set_flags(PANEL_APPLET(applet), PANEL_APPLET_EXPAND_MINOR);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);

  gtk_widget_set_events(GTK_WIDGET(applet), gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),  applet);

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

  priv->action_group = NULL;
  priv->image = NULL;
  priv->timerbox_control = NULL;
  priv->alive = FALSE;

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

PANEL_APPLET_IN_PROCESS_FACTORY("WorkraveAppletFactory",
                                WORKRAVE_TYPE_APPLET,
                                applet_factory,
                                NULL)
