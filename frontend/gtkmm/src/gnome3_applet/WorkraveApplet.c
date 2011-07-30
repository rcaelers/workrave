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

#include "credits.h"

#include <panel-applet.h>

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "WorkraveApplet.h"
#include "applet-server-bindings.h"
#include "gui-client-bindings.h"

#include "nls.h"

struct _WorkraveAppletPrivate
{
  GtkActionGroup *action_group;

  GtkWidget *hbox;
  GtkWidget *event_box;
  GtkWidget *image;
  GtkWidget *socket;
  PanelApplet *applet;
  gboolean has_alpha;

  int size;
  int orientation;

  gboolean last_showlog_state;
  gboolean last_reading_mode_state;
  int last_mode;

  DBusGProxy *support;
  DBusGProxy *ui;
  DBusGProxy *core;
};

G_DEFINE_TYPE (WorkraveApplet, workrave_applet, PANEL_TYPE_APPLET);

static DBusGConnection *g_connection = NULL;

static void workrave_applet_set_all_visible(WorkraveAppletPrivate *priv, gboolean visible);
static void workrave_applet_set_visible(WorkraveAppletPrivate *priv, gchar *name, gboolean visible);
static void workrave_applet_fill(WorkraveApplet *applet);

static void workrave_applet_destroy(GtkWidget *widget);

/************************************************************************/
/* DBUS                                                                 */
/************************************************************************/

static void
workrave_dbus_server_init(WorkraveApplet *applet)
{
  DBusGProxy *driver_proxy;
  GError *err = NULL;
  guint request_name_result;

  g_return_if_fail(g_connection == NULL);
  g_return_if_fail(applet->priv != NULL);

  g_connection = dbus_g_bus_get(DBUS_BUS_SESSION, &err);
  if (g_connection == NULL)
    {
      g_warning("DBUS Service registration failed: %s", err ? err->message : "");
      g_error_free(err);
      return;
    }

  dbus_connection_set_exit_on_disconnect(dbus_g_connection_get_connection(g_connection),
                                         FALSE);

  driver_proxy = dbus_g_proxy_new_for_name(g_connection,
                                           DBUS_SERVICE_DBUS,
                                           DBUS_PATH_DBUS,
                                           DBUS_INTERFACE_DBUS);

  if (!org_freedesktop_DBus_request_name(driver_proxy,
                                         DBUS_SERVICE_APPLET,
                                         0,
                                         &request_name_result,
                                         &err))
    {
      g_warning("DBUS Service name request failed.");
      g_clear_error(&err);
    }

  if (request_name_result == DBUS_REQUEST_NAME_REPLY_EXISTS)
    {
      g_warning("DBUS Service already started elsewhere");
      return;
    }

  dbus_g_object_type_install_info(WORKRAVE_TYPE_APPLET,
                                  &dbus_glib_workrave_object_info);

  dbus_g_connection_register_g_object(g_connection,
                                      "/org/workrave/Workrave/GnomeApplet",
                                      G_OBJECT(applet));

  applet->priv->support = dbus_g_proxy_new_for_name(g_connection,
                                                "org.workrave.Workrave.Activator",
                                                "/org/workrave/Workrave/UI",
                                                "org.workrave.GnomeAppletSupportInterface");

  applet->priv->ui = dbus_g_proxy_new_for_name(g_connection,
                                           "org.workrave.Workrave.Activator",
                                           "/org/workrave/Workrave/UI",
                                           "org.workrave.ControlInterface");

  applet->priv->core = dbus_g_proxy_new_for_name(g_connection,
                                             "org.workrave.Workrave.Activator",
                                             "/org/workrave/Workrave/Core",
                                             "org.workrave.CoreInterface");
}


static void
workrave_dbus_server_cleanup()
{
  DBusGProxy *driver_proxy;
  GError *err = NULL;
  guint release_name_result;

  driver_proxy = dbus_g_proxy_new_for_name(g_connection,
                                           DBUS_SERVICE_DBUS,
                                           DBUS_PATH_DBUS,
                                           DBUS_INTERFACE_DBUS);

  if (!org_freedesktop_DBus_release_name(driver_proxy,
                                         DBUS_SERVICE_APPLET,
                                         &release_name_result,
                                         &err))
    {
      g_warning("DBUS Service name release failed.");
      g_clear_error(&err);
    }

  if (g_connection != NULL)
    {
      dbus_g_connection_unref(g_connection);
      g_connection = NULL;
    }
}


static gboolean
workrave_is_running(void)
{
	DBusGProxy *dbus = NULL;
	GError *error = NULL;
	gboolean running = FALSE;

  if (g_connection != NULL)
    {
      dbus = dbus_g_proxy_new_for_name(g_connection,
                                       "org.freedesktop.DBus",
                                       "/org/freedesktop/DBus",
                                       "org.freedesktop.DBus");
    }

  if (dbus != NULL)
    {
      dbus_g_proxy_call(dbus, "NameHasOwner", &error,
                        G_TYPE_STRING, "org.workrave.Workrave",
                        G_TYPE_INVALID,
                        G_TYPE_BOOLEAN, &running,
                        G_TYPE_INVALID);
    }

	return running;
}


gboolean
workrave_applet_get_socket_id(WorkraveApplet *applet, guint *id, GError **err)
{
  *id = gtk_socket_get_id(GTK_SOCKET(applet->priv->socket));

  return TRUE;
}


gboolean
workrave_applet_get_size(WorkraveApplet *applet, guint *size, GError **err)
{
  *size = applet->priv->size;

  return TRUE;
}


gboolean
workrave_applet_get_orientation(WorkraveApplet *applet, guint *orientation, GError **err)
{
  *orientation = applet->priv->orientation;

  return TRUE;
}


gboolean
workrave_applet_set_menu_status(WorkraveApplet *applet, const char *name, gboolean status, GError **err)
{
  if (g_str_has_prefix(name, "/commands/"))
    {
      name += 10; // Skip gnome2 prefix for compatibility
    }

  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      gtk_toggle_action_set_active(toggle, status);

      return TRUE;
    }

  return FALSE;
}


gboolean
workrave_applet_get_menu_status(WorkraveApplet *applet, const char *name,  gboolean *status, GError **err)
{
  if (g_str_has_prefix(name, "/commands/"))
    {
      name += 10; // Skip gnome2 prefix for compatibility
    }

  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      *status = gtk_toggle_action_get_active(toggle);

      return TRUE;
    }
  return FALSE;
}


gboolean
workrave_applet_set_menu_active(WorkraveApplet *applet, const char *name, gboolean status, GError **err)
{
  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  gtk_action_set_visible(action, status);

  return TRUE;
}


gboolean
workrave_applet_get_menu_active(WorkraveApplet *applet, const char *name, gboolean *active, GError **err)
{
  GtkAction *action = gtk_action_group_get_action(applet->priv->action_group, name);
  *active = gtk_action_get_visible(action);

  return TRUE;
}



/************************************************************************/
/* GNOME::Applet                                                        */
/************************************************************************/

static void
dbus_callback(DBusGProxy *proxy,
              DBusGProxyCall *call,
              void *user_data)
{
  GError *error = NULL;

  dbus_g_proxy_end_call(proxy, call, &error, G_TYPE_INVALID);

  if (error != NULL)
    {
      g_warning("DBUS Failed: %s", error ? error->message : "");
      g_error_free(error);
    }
}


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
on_menu_open(GtkAction *action, WorkraveApplet *applet)
{
  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "OpenMain", dbus_callback, NULL, NULL,
                              G_TYPE_INVALID);
    }
}


static void
on_menu_preferences(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "Preferences", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}


static void
on_menu_exercises(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "Exercises", dbus_callback, NULL, NULL,
                              G_TYPE_INVALID);
    }
}

static void
on_menu_statistics(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "Statistics", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}

static void
on_menu_restbreak(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "RestBreak", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}



static void
on_menu_connect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "NetworkConnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);

    }
}


static void
on_menu_disconnect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "NetworkDisconnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}

static void
on_menu_reconnect(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "NetworkReconnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}



static void
on_menu_quit(GtkAction *action, WorkraveApplet *applet)
{
  if (!workrave_is_running())
    {
      return;
    }

  if (applet->priv->ui != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "Quit", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}


static gboolean
plug_removed(GtkSocket *socket, WorkraveAppletPrivate *priv)
{
  gtk_widget_show(GTK_WIDGET(priv->image));
  gtk_widget_hide(GTK_WIDGET(priv->socket));
  workrave_applet_set_all_visible(priv, FALSE);
  return TRUE;
}


static gboolean
plug_added(GtkSocket *socket, WorkraveAppletPrivate *priv)
{
  gtk_widget_hide(GTK_WIDGET(priv->image));
  gtk_widget_show(GTK_WIDGET(priv->socket));
  workrave_applet_set_all_visible(priv, TRUE);

  return TRUE;
}


static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, WorkraveAppletPrivate *priv)
{
  gboolean ret = FALSE;

  if (event->button == 1)
    {
      if (priv->support != NULL && workrave_is_running())
        {
          dbus_g_proxy_begin_call(priv->support, "ButtonClicked", dbus_callback, NULL, NULL,
                                  G_TYPE_UINT, event->button, G_TYPE_INVALID);

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

  if (applet->priv->ui != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "NetworkLog", dbus_callback, NULL, NULL,
                              G_TYPE_BOOLEAN, new_state, G_TYPE_INVALID);
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

  if (applet->priv->ui != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(applet->priv->ui, "ReadingMode", dbus_callback, NULL, NULL,
                              G_TYPE_BOOLEAN, new_state, G_TYPE_INVALID);
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

      if (applet->priv->core != NULL && workrave_is_running())
        {
          dbus_g_proxy_begin_call(applet->priv->core, "SetOperationMode", dbus_callback, NULL, NULL,
                                  G_TYPE_STRING, modes[mode], G_TYPE_INVALID);
        }
    }
}


static void
workrave_applet_set_visible(WorkraveAppletPrivate *priv, gchar *name, gboolean visible)
{
  GtkAction *action;

  action = gtk_action_group_get_action(priv->action_group, name);
  gtk_action_set_visible(action, visible);
}


static void
workrave_applet_set_all_visible(WorkraveAppletPrivate *priv, gboolean visible)
{
  workrave_applet_set_visible(priv, "Preferences", visible);
  workrave_applet_set_visible(priv, "Restbreak", visible);
  workrave_applet_set_visible(priv, "Network", visible);
  workrave_applet_set_visible(priv, "Normal", visible);
  workrave_applet_set_visible(priv, "Suspended", visible);
  workrave_applet_set_visible(priv, "Quiet", visible);
  workrave_applet_set_visible(priv, "Mode", visible);
  workrave_applet_set_visible(priv, "Statistics", visible);
  workrave_applet_set_visible(priv, "Exercises", visible);
  workrave_applet_set_visible(priv, "ReadingMode", visible);
  workrave_applet_set_visible(priv, "Quit", visible);
}


static
void workrave_applet_destroy(GtkWidget *widget)
{
  workrave_dbus_server_cleanup();
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
  // GdkVisual *visual = gtk_widget_get_visual(widget);

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
  workrave_dbus_server_cleanup();
  GTK_WIDGET_CLASS(workrave_applet_parent_class)->unrealize (widget);
}


static void
workrave_applet_change_background(PanelApplet *panel_applet, cairo_pattern_t *pattern)
{
  // WorkraveApplet *applet = WORKRAVE_APPLET(panel);
  // workrave_force_redraw(applet);
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

  if (applet->priv->support != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(applet->priv->support, "SetOrientation", dbus_callback, NULL, NULL,
                              G_TYPE_STRING, str, G_TYPE_INVALID);
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
  widget_class->destroy = workrave_applet_destroy;

  applet_class->change_background = workrave_applet_change_background;
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
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),  applet->priv);

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

  priv->image = NULL;
  priv->socket = NULL;
  priv->size = 48;
  priv->orientation = 0;
  priv->last_showlog_state = FALSE;
  priv->last_reading_mode_state = FALSE;
  priv->last_mode = 0;
  priv->support = NULL;
  priv->ui = NULL;
  priv->core = NULL;

  workrave_dbus_server_init(applet);
  workrave_applet_fill(applet);

  if (applet->priv->support != NULL)
    {
      dbus_g_proxy_begin_call(applet->priv->support, "EmbedRequest", dbus_callback, NULL, NULL,
                              G_TYPE_INVALID);
    }

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
