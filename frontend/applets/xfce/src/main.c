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

#ifdef PANEL_MATE
#include <mate-panel-applet.h>
#endif
#ifdef PANEL_XFCE4
#include <libxfce4panel/xfce-panel-plugin.h>
#endif

#include <gio/gio.h>

#include "control.h"
#include "credits.h"
#include "nls.h"

#include "MenuEnums.hh"

typedef struct _WorkraveApplet
{
#ifdef PANEL_XFCE4
  XfcePanelPlugin* plugin;
#endif
  
  WorkraveTimerboxControl *timerbox_control;
  GtkImage *image;
  gboolean alive;
  int inhibit;
  GtkMenuItem *menu_items[MENU_COMMAND_SIZEOF];

} WorkraveApplet;

static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);
static void on_menu_command(GtkMenuItem *menuitem, gpointer user_data);
static void on_menu_check_changed(GtkMenuItem *item, gpointer user_data);
static void on_menu_radio_changed(GtkMenuItem *item, gpointer user_data);
static void on_menu_about(GtkMenuItem *item, WorkraveApplet *applet);
static void on_menu_mode_changed(const char *mode, WorkraveApplet *applet);

struct Menuitems
{
  enum MenuCommand id;
  gboolean autostart;
  char *dbuscmd;
  gboolean 
};

static struct Menuitems menu_data[] =
  {
    { MENU_COMMAND_OPEN,                  TRUE,  "OpenMain"          },
    { MENU_COMMAND_PREFERENCES,           FALSE, "Preferences"       },
    { MENU_COMMAND_EXERCISES,             FALSE, "Exercises"         },
    { MENU_COMMAND_REST_BREAK,            FALSE, "RestBreak"         },
    { MENU_COMMAND_MODE_SUBMENU,          FALSE, NULL                },
    { MENU_COMMAND_MODE_NORMAL,           FALSE, NULL                },
    { MENU_COMMAND_MODE_QUIET,            FALSE, NULL                },
    { MENU_COMMAND_MODE_SUSPENDED,        FALSE, NULL                },
    { MENU_COMMAND_NETWORK_SUBMENU,       FALSE, NULL                },
    { MENU_COMMAND_NETWORK_CONNECT,       FALSE, "NetworkConnect"    },
    { MENU_COMMAND_NETWORK_DISCONNECT,    FALSE, "NetworkDisconnect" },
    { MENU_COMMAND_NETWORK_LOG,           FALSE, "NetworkLog"        },
    { MENU_COMMAND_NETWORK_RECONNECT,     FALSE, "NetworkReconnect"  },
    { MENU_COMMAND_STATISTICS,            FALSE, "Statistics"        },
    { MENU_COMMAND_ABOUT,                 FALSE, NULL                },
    { MENU_COMMAND_MODE_READING,          FALSE, "ReadingMode"       },
    { MENU_COMMAND_QUIT,                  FALSE, "Quit"              }
  };

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
lookup_menu_index_by_menu_item(WorkraveApplet *applet, GtkMenuItem *item)
{
  for (int i = 0; i < MENU_COMMAND_SIZEOF; i++)
    {
      if (applet->menu_items[i] == item)
        {
          return lookup_menu_index_by_id(i);
        }
    }

  return -1;
}


void on_alive_changed(gpointer instance, gboolean alive, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet*)user_data;
  applet->alive = alive;

  if (!alive)
    {
      for (int i = 0; i < sizeof(menu_data)/sizeof(struct Menuitems); i++)
        {
          GtkMenuItem *item = applet->menu_items[i];
          if (item != NULL)
            {
              gtk_widget_set_visible(GTK_WIDGET(item), FALSE);
            }
        }
    }
}

void on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet*)user_data;

  GVariantIter *iter;
  g_variant_get (parameters, "(a(sii))", &iter);

  printf("on_menu_changed\n");
  
  char *text;
  int id;
  int flags;

  GSList *radio_group = NULL;
  GtkWidget *menu = NULL;
  while (g_variant_iter_loop(iter, "(sii)", &text, &id, &flags))
    {
      printf("on_menu_changed: %s %d %d\n", text, id, flags);
      int index = lookup_menu_index_by_id((enum MenuCommand)id);
      if (index == -1)
        {
          continue;
        }

      GtkMenuItem *item = applet->menu_items[id];

      if (flags & MENU_ITEM_FLAG_SUBMENU_END)
        {
          radio_group = NULL;
          menu = NULL;
        }
      
      else if (item == NULL)
        {
          printf("on_menu_changed add\n");
          
          if (flags & MENU_ITEM_FLAG_SUBMENU_BEGIN)
            {
              menu = gtk_menu_new();
              item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(text));
              gtk_menu_item_set_submenu(item, menu);
            }
          else if (flags & MENU_ITEM_FLAG_RADIO)
            {
              item = GTK_MENU_ITEM(gtk_radio_menu_item_new_with_label(radio_group, text));
              radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
              g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_radio_changed), applet);
            }
          else if (flags & MENU_ITEM_FLAG_CHECK)
            {
              item = GTK_MENU_ITEM(gtk_check_menu_item_new_with_label(text));
              g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_check_changed), applet);
            }
          else
            {
              item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(text));
              g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_command), applet);
            }
          
          if (item == NULL)
            {
              continue;
            }
          
          printf("on_menu_changed add2x\n");

          applet->menu_items[id] = item;
          if (menu != NULL && !(flags & MENU_ITEM_FLAG_SUBMENU_BEGIN))
            {
              gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));
            }
          else if (!(flags & MENU_ITEM_FLAG_SUBMENU_END))
            {
              xfce_panel_plugin_menu_insert_item(applet->plugin, item);
            }
          gtk_widget_show(GTK_WIDGET(item));
        }
      
      if (item != NULL)
        {
          applet->inhibit++;
          if (GTK_IS_CHECK_MENU_ITEM(item))
            {
              gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), flags & MENU_ITEM_FLAG_ACTIVE);
            }
          applet->inhibit--;
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

static void
on_menu_command(GtkMenuItem *item, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet*)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  printf("on_menu_command\n");
  
  int index = lookup_menu_index_by_menu_item(applet, item);
  if (index == -1)
    {
      return;
    }

  printf("on_menu_command : %d %d\n", index, menu_data[index].id);

  switch(menu_data[index].id)
    {
    case MENU_COMMAND_ABOUT:
      on_menu_about(item, applet);
      break;

    default:
      {
        GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->timerbox_control);
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
      break;
    }
}

static void
on_menu_about(GtkMenuItem *item, WorkraveApplet *applet)
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
on_menu_check_changed(GtkMenuItem *item, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet*)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  int index = lookup_menu_index_by_menu_item(applet, item);
  if (index == -1)
    {
      return;
    }

  gboolean new_state = FALSE;
  if (GTK_IS_CHECK_MENU_ITEM(item))
    {
      new_state = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(item));
    }

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        menu_data[index].dbuscmd,
                        g_variant_new("(b)", new_state),
                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                        -1,
                        NULL,
                        (GAsyncReadyCallback) dbus_call_finish,
                        applet);
    }
}

static void
on_menu_radio_changed(GtkMenuItem *item, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet*)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  int index = lookup_menu_index_by_menu_item(applet,item);
  if (index == -1)
    {
      return;
    }

  switch(menu_data[index].id)
    {
    case MENU_COMMAND_MODE_NORMAL:
      on_menu_mode_changed("normal", applet);
      break;
    case MENU_COMMAND_MODE_SUSPENDED:
      on_menu_mode_changed("suspended", applet);
      break;
    case MENU_COMMAND_MODE_QUIET:
      on_menu_mode_changed("quiet", applet);
      break;
    default:
      break;
    }
}

static void
on_menu_mode_changed(const char *mode, WorkraveApplet *applet)
{
  GDBusProxy *proxy = workrave_timerbox_control_get_core_proxy(applet->timerbox_control);
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

#ifdef PANEL_MATE
static gboolean workrave_applet_fill(MatePanelApplet *applet)
#endif
#ifdef PANEL_XFCE4
static void workrave_applet_fill(WorkraveApplet *applet)
#endif
{
#ifdef PANEL_MATE
  mate_panel_applet_set_flags(applet,
                              MATE_PANEL_APPLET_EXPAND_MAJOR |
                              MATE_PANEL_APPLET_EXPAND_MINOR |
                              MATE_PANEL_APPLET_HAS_HANDLE);

  mate_panel_applet_set_background_widget(applet, GTK_WIDGET(applet));
#endif

  applet->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->image = workrave_timerbox_control_get_image(applet->timerbox_control);
  g_signal_connect(G_OBJECT(applet->timerbox_control), "menu-changed", G_CALLBACK(on_menu_changed),  applet);
  g_signal_connect(G_OBJECT(applet->timerbox_control), "alive-changed", G_CALLBACK(on_alive_changed),  applet);

  workrave_timerbox_control_show_tray_icon_when_not_running(applet->timerbox_control, TRUE);

#ifndef PANEL_XFCE4
  gtk_container_add(GTK_CONTAINER(applet), applet->image);
  gtk_widget_show_all(GTK_WIDGET(applet));
  return TRUE;
#else
  gtk_container_add(GTK_CONTAINER(applet->plugin), GTK_WIDGET(applet->image));
#endif
}

#ifdef PANEL_MATE
static gboolean workrave_applet_factory(MatePanelApplet *applet, const gchar *iid, gpointer data)
{
  gboolean retval = FALSE;
  
  if (!g_strcmp0(iid, "WorkraveApplet") == 0)
    {
      retval = workrave_applet_fill(applet);
    }
    
  if (!retval)
    {
      exit(-1);
    }
    
  return retval;
}
#endif

#ifdef PANEL_XFCE4
static void workrave_applet_construct(XfcePanelPlugin *plugin)
{
  WorkraveApplet *applet = panel_slice_new0(WorkraveApplet);
  applet->plugin = plugin;
  applet->image = NULL;
  applet->timerbox_control = NULL;
  applet->alive = FALSE;
  applet->inhibit = 0;
  
  printf("workrave_applet_construct\n");
  g_debug("workrave_applet_construct\n");
  
  for (int i = 0; i < MENU_COMMAND_SIZEOF;i ++)
    {
      applet->menu_items[i] = NULL;
    }
  
  workrave_applet_fill(applet);
  xfce_panel_plugin_set_expand(plugin, TRUE);
  gtk_widget_show_all(GTK_WIDGET(plugin));
}
#endif

#ifdef PANEL_MATE
MATE_PANEL_APPLET_OUT_PROCESS_FACTORY(
    "WorkraveAppletFactory",
    PANEL_TYPE_APPLET,
    "WorkraveApplet",
    workrave_applet_factory,
    NULL);
#endif

#ifdef PANEL_XFCE4
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(workrave_applet_construct);
#endif
