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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "WorkraveApplet.h"
#include "applet-server-bindings.h"
#include "gui-client-bindings.h"

#include "nls.h"

G_DEFINE_TYPE (WorkraveApplet, workrave_applet, G_TYPE_OBJECT);

static WorkraveApplet *g_applet = NULL;
static DBusGConnection *g_connection = NULL;

static void workrave_applet_set_all_visible(gboolean visible);
static void workrave_applet_set_visible(gchar *name, gboolean visible);

static
void workrave_applet_destroy(GtkObject *object);

/************************************************************************/
/* DBUS                                                                 */
/************************************************************************/

static WorkraveApplet*
workrave_applet_new()
{
  return (WorkraveApplet *)g_object_new(WORKRAVE_APPLET_TYPE, NULL);
}

static void
workrave_applet_class_init(WorkraveAppletClass *klass)
{
}


static void
workrave_applet_init(WorkraveApplet *applet)
{
  applet->image = NULL;
  applet->socket = NULL;
  applet->size = 48;
  applet->socket_id = 0;
  applet->orientation = 0;
  applet->last_showlog_state = FALSE;
  applet->last_reading_mode_state = FALSE;
  applet->last_mode = 0; 
  applet->applet = NULL;
  
  applet->support = NULL;
  applet->ui = NULL;
  applet->core = NULL;
}


static void
workrave_dbus_server_init()
{
  DBusGProxy *driver_proxy;
  GError *err = NULL;
  guint request_name_result;

  g_return_if_fail(g_connection == NULL);
  g_return_if_fail(g_applet != NULL);

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

  dbus_g_object_type_install_info(WORKRAVE_APPLET_TYPE,
                                  &dbus_glib_workrave_object_info);

  dbus_g_connection_register_g_object(g_connection,
                                      "/org/workrave/Workrave/GnomeApplet",
                                      G_OBJECT(g_applet));

  g_applet->support = dbus_g_proxy_new_for_name(g_connection,
                                                "org.workrave.Workrave.Activator",
                                                "/org/workrave/Workrave/UI",
                                                "org.workrave.GnomeAppletSupportInterface");

  g_applet->ui = dbus_g_proxy_new_for_name(g_connection,
                                           "org.workrave.Workrave.Activator",
                                           "/org/workrave/Workrave/UI",
                                           "org.workrave.ControlInterface");

  g_applet->core = dbus_g_proxy_new_for_name(g_connection,
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
  *id = applet->socket_id;

  return TRUE;
}


gboolean
workrave_applet_get_size(WorkraveApplet *applet, guint *size, GError **err)
{
  *size = applet->size;

  return TRUE;
}


gboolean
workrave_applet_get_orientation(WorkraveApplet *applet, guint *orientation, GError **err)
{
  *orientation = applet->orientation;

  return TRUE;
}


gboolean
workrave_applet_set_menu_status(WorkraveApplet *applet, const char *name, gboolean status, GError **err)
{
  if (g_str_has_prefix(name, "/commands/"))
    {
      name += 10; // Skip gnome2 prefix for compatibility
    }

  GtkAction *action = gtk_action_group_get_action(g_applet->action_group, name);
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

  GtkAction *action = gtk_action_group_get_action(g_applet->action_group, name);
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
  GtkAction *action = gtk_action_group_get_action(g_applet->action_group, name);
  gtk_action_set_visible(action, status);

  return TRUE;
}


gboolean
workrave_applet_get_menu_active(WorkraveApplet *applet, const char *name, gboolean *active, GError **err)
{
  GtkAction *action = gtk_action_group_get_action(g_applet->action_group, name);
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
on_menu_about(GtkAction *action, WorkraveApplet *a)
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
on_menu_open(GtkAction *action, WorkraveApplet *a)
{
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "OpenMain", dbus_callback, NULL, NULL,
                              G_TYPE_INVALID);
    }
}


static void
on_menu_preferences(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "Preferences", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}


static void
on_menu_exercises(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "Exercises", dbus_callback, NULL, NULL,
                              G_TYPE_INVALID);
    }
}

static void
on_menu_statistics(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    { 
      dbus_g_proxy_begin_call(g_applet->ui, "Statistics", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}

static void
on_menu_restbreak(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "RestBreak", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}



static void
on_menu_connect(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "NetworkConnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);

    }
}


static void
on_menu_disconnect(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "NetworkDisconnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}

static void
on_menu_reconnect(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "NetworkReconnect", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}



static void
on_menu_quit(GtkAction *action, WorkraveApplet *a)
{
  if (!workrave_is_running())
    {
      return;
    }
  
  if (g_applet->ui != NULL)
    {
      dbus_g_proxy_begin_call(g_applet->ui, "Quit", dbus_callback, NULL, NULL,
                             G_TYPE_INVALID);
    }
}

static void
change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  switch (o)
    {
    case PANEL_APPLET_ORIENT_UP:
      g_applet->orientation = 0;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
      g_applet->orientation = 1;
      break;
    case PANEL_APPLET_ORIENT_DOWN:
      g_applet->orientation = 2;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      g_applet->orientation = 3;
      break;
    }

  if (g_applet->support != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(g_applet->support, "SetOrientation", dbus_callback, NULL, NULL,
                             G_TYPE_UINT, g_applet->orientation, G_TYPE_INVALID);


    }
}

static void
change_background(PanelApplet * widget,
                  PanelAppletBackgroundType type,
                  GdkColor * color,
                  GdkPixmap * pixmap,
                  void *data)
{
  static GdkPixmap *keep = NULL;
  long xid = 0;
  GValueArray *val = g_value_array_new(4);

  if (type == PANEL_NO_BACKGROUND)
    {
      GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(widget));

      if (style->bg_pixmap[GTK_STATE_NORMAL])
        {
          pixmap = style->bg_pixmap[GTK_STATE_NORMAL];
          type = PANEL_PIXMAP_BACKGROUND;
        }
      else
        {
          color = &style->bg[GTK_STATE_NORMAL];
          if (color != NULL)
            {
              type = PANEL_COLOR_BACKGROUND;
            }
        }
    }
  
  if (type == PANEL_COLOR_BACKGROUND && color != NULL)
    {
      g_value_array_append(val, NULL);
      g_value_init(g_value_array_get_nth(val, 0), G_TYPE_UINT);
      g_value_set_uint(g_value_array_get_nth(val, 0), color->pixel);
 
      g_value_array_append(val, NULL);
      g_value_init(g_value_array_get_nth(val, 1), G_TYPE_UINT);
      g_value_set_uint(g_value_array_get_nth(val, 1), color->red);
      
      g_value_array_append(val, NULL);
      g_value_init(g_value_array_get_nth(val, 2), G_TYPE_UINT);
      g_value_set_uint(g_value_array_get_nth(val, 2), color->green);

      g_value_array_append(val, NULL);
      g_value_init(g_value_array_get_nth(val, 3), G_TYPE_UINT);
      g_value_set_uint(g_value_array_get_nth(val, 3), color->blue);
    }
  else
    {
      int i;
      for (i = 0; i < 4; i++)
        {
          g_value_array_prepend(val, NULL);
          g_value_init(g_value_array_get_nth(val, 0), G_TYPE_UINT);
          g_value_set_uint(g_value_array_get_nth(val, 0), 0);
        }
    }

#ifndef HAVE_GTK3
  // TODO: gtk3 port
  if (type == PANEL_PIXMAP_BACKGROUND)
    {
      if (keep != NULL)
        {
          gdk_pixmap_unref(keep);
          keep = pixmap;
        }
      if (pixmap != NULL)
        {
          gdk_pixmap_ref(pixmap);
        }

      xid = GDK_PIXMAP_XID(pixmap);
    }
#endif
  
  if (g_applet->support != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(g_applet->support, "SetBackground", dbus_callback, NULL, NULL,
                              G_TYPE_UINT, type,
                              G_TYPE_VALUE_ARRAY, val,
                              G_TYPE_UINT, xid,
                              G_TYPE_INVALID);
      

    }

  g_value_array_free(val);
}


static gboolean
plug_removed(GtkSocket *socket, void *manager)
{
  gtk_widget_show(GTK_WIDGET(g_applet->image));
  gtk_widget_hide(GTK_WIDGET(g_applet->socket));
  workrave_applet_set_all_visible(FALSE);
  return TRUE;
}


static gboolean
plug_added(GtkSocket *socket, void *manager)
{
  gtk_widget_hide(GTK_WIDGET(g_applet->image));
  gtk_widget_show(GTK_WIDGET(g_applet->socket));
  workrave_applet_set_all_visible(TRUE);
  return TRUE;
}


static gboolean
button_pressed(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  gboolean ret = FALSE;
  
  if (event->button == 1)
    {
      if (g_applet->support != NULL && workrave_is_running())
        {
          dbus_g_proxy_begin_call(g_applet->support, "ButtonClicked", dbus_callback, NULL, NULL,
                                  G_TYPE_UINT, event->button, G_TYPE_INVALID);

          ret = TRUE;
        }
    }

  return ret;
}

static void
showlog_callback(GtkAction *action, WorkraveApplet *a)
{
  gboolean new_state = FALSE;

  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      new_state = gtk_toggle_action_get_active(toggle);
    }

  g_applet->last_showlog_state = new_state;

  if (g_applet->ui != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(g_applet->ui, "NetworkLog", dbus_callback, NULL, NULL,
                              G_TYPE_BOOLEAN, new_state, G_TYPE_INVALID);
    }
}

static void
reading_mode_callback(GtkAction *action, WorkraveApplet *a)
{
  gboolean new_state = FALSE;

  if (GTK_IS_TOGGLE_ACTION(action))
    {
      GtkToggleAction *toggle = GTK_TOGGLE_ACTION(action);
      new_state = gtk_toggle_action_get_active(toggle);
    }

  g_applet->last_reading_mode_state = new_state;
  
  if (g_applet->ui != NULL && workrave_is_running())
    {
      dbus_g_proxy_begin_call(g_applet->ui, "ReadingMode", dbus_callback, NULL, NULL,
                              G_TYPE_BOOLEAN, new_state, G_TYPE_INVALID);
    }
}

static void
mode_callback(GtkAction *action, WorkraveApplet *a)
{
  const char *modes[] = { "normal", "suspended", "quiet" };
  int mode = 0;

  if (GTK_IS_RADIO_ACTION(action))
    {
      GtkRadioAction *toggle = GTK_RADIO_ACTION(action);
      mode = gtk_radio_action_get_current_value(toggle);
    }

  if (mode >= 0 && mode < G_N_ELEMENTS(modes))
    {
      g_applet->last_mode = mode;
          
      if (g_applet->ui != NULL && workrave_is_running())
        {
          dbus_g_proxy_begin_call(g_applet->core, "SetOperationMode", dbus_callback, NULL, NULL,
                                  G_TYPE_STRING, modes[mode], G_TYPE_INVALID);
        }
    }
}


static void
workrave_applet_set_visible(gchar *name, gboolean visible)
{
  GtkAction *action;

  action = gtk_action_group_get_action(g_applet->action_group, name);
  gtk_action_set_visible(action, visible);
}


static void
workrave_applet_set_all_visible(gboolean visible)
{
  workrave_applet_set_visible("Preferences", visible);
  workrave_applet_set_visible("Restbreak", visible);
  workrave_applet_set_visible("Network", visible);
  workrave_applet_set_visible("Normal", visible);
  workrave_applet_set_visible("Suspended", visible);
  workrave_applet_set_visible("Quiet", visible);
  workrave_applet_set_visible("Mode", visible);
  workrave_applet_set_visible("Statistics", visible);
  workrave_applet_set_visible("Exercises", visible);
  workrave_applet_set_visible("ReadingMode", visible);
  workrave_applet_set_visible("Quit", visible);
}


/* stolen from clock applet :) */
static inline void
force_no_focus_padding(GtkWidget *widget)
{
  static gboolean first = TRUE;

  if (first)
    {
      gtk_rc_parse_string ("\n"
                           "   style \"hdate-applet-button-style\"\n"
                           "   {\n"
                           "      GtkWidget::focus-line-width=0\n"
                           "      GtkWidget::focus-padding=0\n"
                           "   }\n"
                           "\n"
                           "    widget \"*.hdate-applet-button\" style \"hdate-applet-button-style\"\n"
                           "\n");
      first = FALSE;
    }

  gtk_widget_set_name(widget, "hdate-applet-button");
}

void size_allocate(GtkWidget     *widget,
                   GtkAllocation *allocation,
                   gpointer       user_data)
{
  static int prev_width = -1;
  static int prev_height = -1;

  if (prev_height == -1 || prev_width == -1 ||
      allocation->width != prev_width ||
      allocation->height != prev_height)
    {
      prev_height = allocation->height;
      prev_width = allocation->width;

      if (g_applet->orientation == 1 ||
          g_applet->orientation == 3)
        {
          g_applet->size = allocation->width;
        }
      else
        {
          g_applet->size = allocation->height;
        }

      if (g_applet->support != NULL && workrave_is_running())
        {
          dbus_g_proxy_begin_call(g_applet->support, "SetSize", dbus_callback, NULL, NULL,
                                  G_TYPE_UINT, g_applet->size,
                                  G_TYPE_INVALID);
      
        }
    }
}
      

static
void workrave_applet_destroy(GtkObject *object)
{
  workrave_dbus_server_cleanup();
}


static void
workrave_applet_realize(GtkWidget *widget, void *data)
{
}


static void
workrave_applet_unrealize(GtkWidget *widget, void *data)
{
  if (g_applet != NULL)
    {
      g_object_unref(g_applet);
      g_applet = NULL;
    }

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


static gboolean
workrave_applet_fill(PanelApplet *applet)
{
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *hbox = NULL;
  PanelAppletOrient orient;

  g_applet->action_group = gtk_action_group_new("WorkraveAppletActions");
  gtk_action_group_set_translation_domain(g_applet->action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions(g_applet->action_group,
                               menu_actions,
                               G_N_ELEMENTS (menu_actions),
                               applet);
  gtk_action_group_add_toggle_actions(g_applet->action_group,
                                      toggle_actions,
                                      G_N_ELEMENTS (toggle_actions),
                                      applet);
  gtk_action_group_add_radio_actions (g_applet->action_group,
                                      mode_actions,
                                      G_N_ELEMENTS(mode_actions),
                                      0,
                                      G_CALLBACK(mode_callback),
                                      NULL);

 gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-applet-menu.xml", NULL);
  panel_applet_setup_menu_from_file(applet, ui_path, g_applet->action_group);
  g_free(ui_path);
  //g_object_unref(action_group);
  
  panel_applet_set_flags(PANEL_APPLET(applet),
                         PANEL_APPLET_EXPAND_MINOR);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);
  panel_applet_set_background_widget(applet, GTK_WIDGET(applet));
  gtk_widget_set_events(GTK_WIDGET(applet),
                        gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);


  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed),
                   g_applet);

  // Socket.
  g_applet->socket = gtk_socket_new();
  gtk_container_set_border_width(GTK_CONTAINER(g_applet->socket), 0);

  // Image
  pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave-icon-medium.png", NULL);
  g_applet->image = gtk_image_new_from_pixbuf(pixbuf);

  // Signals.
  g_signal_connect(g_applet->socket, "plug_removed", G_CALLBACK(plug_removed), NULL);
  g_signal_connect(g_applet->socket, "plug_added", G_CALLBACK(plug_added), NULL);
  g_signal_connect(G_OBJECT(applet), "change_orient", G_CALLBACK(change_orient), NULL);

  // Container.
  hbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(applet), hbox);
  gtk_box_pack_end(GTK_BOX(hbox), g_applet->socket, TRUE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), g_applet->image, TRUE, TRUE, 0);

  gtk_container_set_border_width(GTK_CONTAINER(hbox), 0);

  g_applet->socket_id = gtk_socket_get_id(GTK_SOCKET(g_applet->socket));
  g_applet->size = panel_applet_get_size(applet);

  orient = panel_applet_get_orient(applet);

  switch (orient)
    {
    case PANEL_APPLET_ORIENT_UP:
      g_applet->orientation = 0;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
      g_applet->orientation = 1;
      break;
    case PANEL_APPLET_ORIENT_DOWN:
      g_applet->orientation = 2;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      g_applet->orientation = 3;
      break;
    }

  force_no_focus_padding(GTK_WIDGET(applet));
  force_no_focus_padding(GTK_WIDGET(g_applet->socket));
  force_no_focus_padding(GTK_WIDGET(g_applet->image));
  force_no_focus_padding(GTK_WIDGET(hbox));

  g_signal_connect(G_OBJECT(applet), "destroy", G_CALLBACK(workrave_applet_destroy), NULL);
	g_signal_connect(G_OBJECT(hbox), "realize",   G_CALLBACK(workrave_applet_realize), NULL);
	g_signal_connect(G_OBJECT(hbox), "unrealize", G_CALLBACK(workrave_applet_unrealize), NULL);
  
  gtk_widget_show(GTK_WIDGET(g_applet->image));
  gtk_widget_show(GTK_WIDGET(g_applet->socket));
  gtk_widget_show(GTK_WIDGET(hbox));
  gtk_widget_show(GTK_WIDGET(applet));

  g_signal_connect(G_OBJECT(applet), "size-allocate", G_CALLBACK(size_allocate), NULL);
  g_signal_connect(G_OBJECT(applet), "change_background", G_CALLBACK(change_background), NULL);

  return TRUE;
}


static gboolean
workrave_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;

  if (!g_ascii_strcasecmp(iid, "WorkraveApplet"))
    {
      g_applet = workrave_applet_new();
      g_applet->applet = applet;
      
      workrave_dbus_server_init();
      retval = workrave_applet_fill(applet);

      if (g_applet->support != NULL)
        {
          dbus_g_proxy_begin_call(g_applet->support, "EmbedRequest", dbus_callback, NULL, NULL,
                                  G_TYPE_INVALID);
        }
    }

  return retval;
}

PANEL_APPLET_OUT_PROCESS_FACTORY("WorkraveAppletFactory",
                                 PANEL_TYPE_APPLET,
                                 "Workrave Applet",
                                 workrave_applet_factory,
                                 NULL)
