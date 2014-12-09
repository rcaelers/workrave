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
#include "config.h"
#endif

#include "WorkraveApplet.h"
#include "control.h"
#include "MenuCommand.hh"

#include "credits.h"
#include "nls.h"

#include <panel-applet.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

struct _WorkraveAppletPrivate
{
  GSimpleActionGroup *action_group;
  WorkraveTimerboxControl *timerbox_control;
  GtkImage *image;
};

G_DEFINE_TYPE (WorkraveApplet, workrave_applet, PANEL_TYPE_APPLET);

static void workrave_applet_set_all_visible(WorkraveApplet *applet, gboolean visible);
static void workrave_applet_set_visible(WorkraveApplet *applet, gchar *name, gboolean visible);
static void workrave_applet_fill(WorkraveApplet *applet);
static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);

// TODO: DUPLICATE CODE:
enum MenuItemFlags
  {
    MENU_ITEM_FLAG_NONE = 0,
    MENU_ITEM_FLAG_SUBMENU_BEGIN = 1,
    MENU_ITEM_FLAG_SUBMENU_END = 2,
    MENU_ITEM_FLAG_CHECK = 4,
    MENU_ITEM_FLAG_RADIO = 8,
    MENU_ITEM_FLAG_ACTIVE = 16,
  };

void on_menu_updated(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  GVariantIter *iter;
  g_variant_get (parameters, "(a(sii))", &iter);
  
  char *text;
  int id;
  int flags;
  
  while (g_variant_iter_loop(iter, "(sii)", &text, &id, &flags))  
    {
      // TODO: Cleanup:
      // TODO: check if we have to free text.
      // TODO: disable menus when Workrave is not running.
      switch(id)
        {
        case MENU_COMMAND_MODE_NORMAL:
          {
            if (flags & MENU_ITEM_FLAG_ACTIVE)
              {
                GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), "mode");
                g_simple_action_set_state (G_SIMPLE_ACTION(action), g_variant_new_string("normal"));
              }
          }
          break;
        case MENU_COMMAND_MODE_QUIET:
          {
            if (flags & MENU_ITEM_FLAG_ACTIVE)
              {
                GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), "mode");
                g_simple_action_set_state (G_SIMPLE_ACTION(action), g_variant_new_string("quiet"));
              }
          }
          break;
        case MENU_COMMAND_MODE_SUSPENDED:
          {
            if (flags & MENU_ITEM_FLAG_ACTIVE)
              {
                GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), "mode");
                g_simple_action_set_state (G_SIMPLE_ACTION(action), g_variant_new_string("suspended"));
              }
          }
          break;
        case MENU_COMMAND_NETWORK_LOG:
          {
            GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), "showlog");
            g_simple_action_set_state (G_SIMPLE_ACTION(action), g_variant_new_boolean(flags & MENU_ITEM_FLAG_ACTIVE));
          }
          break;
        case MENU_COMMAND_MODE_READING:
          {
            GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), "readingmode");
            g_simple_action_set_state (G_SIMPLE_ACTION(action), g_variant_new_boolean(flags & MENU_ITEM_FLAG_ACTIVE));
          }
          break;
        }
    }
  
  g_variant_iter_free (iter);
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

/************************************************************************/
/* GNOME::Applet                                                        */
/************************************************************************/

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
menu_call_and_start(WorkraveApplet *applet, char *call)
{
  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        call,
                        NULL,
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        applet);
    }
}

static void
menu_call(WorkraveApplet *applet, char *call)
{
  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                    call,
                    NULL,
                    G_DBUS_CALL_FLAGS_NO_AUTO_START,
                    -1,
                    NULL,
                    (GAsyncReadyCallback) dbus_call_finish,
                    applet);
    }
}

static void
on_menu_open(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call_and_start(applet, "OpenMain");
}

static void
on_menu_preferences(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "Preferences");
}

static void
on_menu_exercises(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "Exercises");
}

static void
on_menu_statistics(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "Statistics");
}

static void
on_menu_restbreak(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "RestBreak");
}

static void
on_menu_connect(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "NetworkConnect");
}

static void
on_menu_disconnect(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "NetworkDisconnect");
}

static void
on_menu_reconnect(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "NetworkReconnect");
}

static void
on_menu_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  menu_call(applet, "Quit");
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

static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, WorkraveApplet *applet)
{
  gboolean ret = FALSE;

  // TODO: 
  if (event->button == 1)
    {
      /* if (applet->priv->support != NULL) */
      /*   { */
          /* g_dbus_proxy_call(applet->priv->support, */
          /*                   "ButtonClicked", */
          /*                   g_variant_new("(u)", event->button), */
          /*                   G_DBUS_CALL_FLAGS_NO_AUTO_START, */
          /*                   -1, */
          /*                   NULL, */
          /*                   (GAsyncReadyCallback) dbus_call_finish, */
          /*                   &applet); */
        /*   ret = TRUE; */
        /* } */
    }

  return ret;
}

static void
showlog_callback(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  g_simple_action_set_state(action, value);

  gboolean new_state = g_variant_get_boolean(value);

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "NetworkLog",
                        g_variant_new("(b)", new_state),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        applet);
    }
}


static void
reading_mode_callback(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  g_simple_action_set_state(action, value);

  gboolean new_state = g_variant_get_boolean(value);

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "ReadingMode",
                        g_variant_new("(b)", new_state),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        &applet);
    }
}

static void
mode_callback(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  g_simple_action_set_state(action, value);

  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  const gchar *mode = g_variant_get_string(value, 0);

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


static void
workrave_applet_set_visible(WorkraveApplet *applet, gchar *name, gboolean visible)
{
  GAction *action = g_action_map_lookup_action(G_ACTION_MAP(applet->priv->action_group), name);
  g_simple_action_set_enabled(G_SIMPLE_ACTION(action), visible);
}

static void
workrave_applet_set_all_visible(WorkraveApplet *applet, gboolean visible)
{
  workrave_applet_set_visible(applet, "preferences", visible);
  workrave_applet_set_visible(applet, "restbreak", visible);
  workrave_applet_set_visible(applet, "mode", visible);
  workrave_applet_set_visible(applet, "statistics", visible);
  workrave_applet_set_visible(applet, "exercises", visible);
  workrave_applet_set_visible(applet, "readingmode", visible);
  workrave_applet_set_visible(applet, "quit", visible);
}

static const GActionEntry menu_actions [] = {
  { "open",        on_menu_open        },
  { "statistics",  on_menu_statistics  },
  { "preferences", on_menu_preferences },
  { "exercises",   on_menu_exercises   },
  { "restbreak",   on_menu_restbreak   },
  { "mode",        on_menu_mode, "s", "'normal'", mode_callback },
  { "join",        on_menu_connect     },
  { "disconnect",  on_menu_disconnect  },
  { "reconnect",   on_menu_reconnect   },
  { "showlog",     on_menu_toggle, NULL, "false", showlog_callback },
  { "readingmode", on_menu_toggle, NULL, "false", reading_mode_callback },
  { "about",       on_menu_about       },
  { "quit",        on_menu_quit        },
};

// TODO: still needed?
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

  g_type_class_add_private(class, sizeof(WorkraveAppletPrivate));
}

static void
workrave_applet_fill(WorkraveApplet *applet)
{
  applet->priv->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->priv->image = workrave_timerbox_control_get_image(applet->priv->timerbox_control);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "menu-updated", G_CALLBACK(on_menu_updated),  applet);
  
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
  panel_applet_set_background_widget(PANEL_APPLET(applet), GTK_WIDGET(applet));

  force_no_focus_padding(GTK_WIDGET(applet));
  force_no_focus_padding(GTK_WIDGET(applet->priv->image));

  gtk_widget_set_events(GTK_WIDGET(applet), gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),  applet);

  gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(applet->priv->image));

  gtk_widget_show(GTK_WIDGET(applet->priv->image));
  gtk_widget_show(GTK_WIDGET(applet));

  // TODO: orientation?
}

static void
workrave_applet_init(WorkraveApplet *applet)
{
  applet->priv = G_TYPE_INSTANCE_GET_PRIVATE(applet, WORKRAVE_TYPE_APPLET, WorkraveAppletPrivate);

  WorkraveAppletPrivate *priv = applet->priv;

  priv->action_group = NULL;
  priv->image = NULL;
  
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
