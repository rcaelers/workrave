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

#include <gdk/gdk.h>
#include <libxfce4panel/libxfce4panel.h>

#include <gio/gio.h>

#include "control.h"
#include "utils.h"
#include "commonui/credits.h"
#include "commonui/MenuDefs.hh"

typedef struct WorkraveApplet
{
  XfcePanelPlugin *plugin;

  WorkraveTimerboxControl *timerbox_control;
  GtkImage *image;
  gboolean alive;
  int inhibit;
  GHashTable *menus;
} WorkraveApplet;

static void dbus_call_finish(GDBusProxy *proxy, GAsyncResult *res, gpointer user_data);
static void on_menu_command(GtkMenuItem *menuitem, gpointer user_data);
static void on_menu_check_changed(GtkMenuItem *item, gpointer user_data);
static void on_menu_radio_changed(GtkMenuItem *item, gpointer user_data);
static void on_menu_about(GtkMenuItem *item, WorkraveApplet *applet);
static void on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data);
static void on_menu_item_changed(gpointer instance, GVariant *parameters, gpointer user_data);

static void
menu_items_add(WorkraveApplet *applet, uint32_t id, GtkMenuItem *item)
{
  uint64_t key = id;

#if GLIB_CHECK_VERSION(2, 68, 0)
  g_hash_table_insert(applet->menus, g_memdup2(&key, sizeof(key)), item);
#else
  g_hash_table_insert(applet->menus, g_memdup(&key, sizeof(key)), item);
#endif
}

/* static void */
/* menu_items_remove_all(WorkraveApplet *applet) */
/* { */
/*   g_hash_table_remove_all(applet->menus); */
/* } */

static GtkMenuItem *
menu_items_lookup_menu_item_by_id(WorkraveApplet *applet, uint32_t id)
{
  uint64_t key = id;
  gpointer p = g_hash_table_lookup(applet->menus, &key);
  if (p != NULL)
    {
      return p;
    }
  return NULL;
}

static gboolean
menu_items_lookup_id_by_menu_item(WorkraveApplet *applet, GtkMenuItem *item, uint32_t *id)
{
  GHashTableIter iter;
  gpointer key = NULL;
  gpointer value = NULL;

  g_hash_table_iter_init(&iter, applet->menus);
  while (g_hash_table_iter_next(&iter, &key, &value))
    {
      if (value == item)
        {
          *id = *((uint64_t *)key);
          return TRUE;
        }
    }
  return FALSE;
}

static void
on_alive_changed(gpointer instance, gboolean alive, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet *)user_data;
  applet->alive = alive;

  if (alive)
    {
      GVariant *menus = workrave_timerbox_control_get_menus(applet->timerbox_control);
      on_menu_changed(NULL, menus, user_data);
      g_variant_unref(menus);
    }
  else
    {
      GHashTableIter iter;
      gpointer key, value;
      g_hash_table_iter_init(&iter, applet->menus);
      while (g_hash_table_iter_next(&iter, &key, &value))
        {
          gtk_widget_hide(GTK_WIDGET(value));
        }
      // TODO: get this to work reliably
      // xfce_panel_plugin_menu_destroy(applet->plugin);
    }
}

static void
update_menu_item(WorkraveApplet *applet, GtkMenuItem *item, char *text, uint8_t flags)
{
  applet->inhibit++;
  if (GTK_IS_CHECK_MENU_ITEM(item))
    {
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), flags & MENU_ITEM_FLAG_ACTIVE);
    }
  gtk_widget_set_visible(GTK_WIDGET(item), (flags & MENU_ITEM_FLAG_VISIBLE));
  gtk_menu_item_set_label(GTK_MENU_ITEM(item), text);

  applet->inhibit--;
  gtk_widget_show(GTK_WIDGET(item));
}

static void
on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet *)user_data;

  // TODO: get this to work reliably
  /* if (g_hash_table_size (applet->menus) > 0) */
  /*   { */
  /*     xfce_panel_plugin_menu_destroy(applet->plugin); */
  /*     menu_items_remove_all(applet); */
  /*   } */

  GVariantIter *iter;
  g_variant_get(parameters, "(a(sssuyy))", &iter);

  char *text;
  char *dynamic_text;
  char *action;
  uint32_t id;
  uint8_t type;
  uint8_t flags;

  GSList *radio_group = NULL;
  GtkWidget *menu = NULL;
  while (g_variant_iter_loop(iter, "(sssuyy)", &text, &dynamic_text, &action, &id, &type, &flags))
    {
      gchar *label;
      gchar shortcut;
      workrave_extract_shortcut(text, &label, &shortcut);

      GtkMenuItem *item = menu_items_lookup_menu_item_by_id(applet, id);
      if (item != NULL && type != MENU_ITEM_TYPE_SUBMENU_END && type != MENU_ITEM_TYPE_RADIOGROUP_END)
        {
          update_menu_item(applet, item, label, flags);
          continue;
        }

      if (type == MENU_ITEM_TYPE_SUBMENU_END)
        {
          radio_group = NULL;
          menu = NULL;
        }
      else if (type == MENU_ITEM_TYPE_SUBMENU_BEGIN)
        {
          menu = gtk_menu_new();
          item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(label));
          gtk_menu_item_set_submenu(item, menu);
        }
      else if (type == MENU_ITEM_TYPE_RADIO)
        {
          item = GTK_MENU_ITEM(gtk_radio_menu_item_new_with_label(radio_group, label));
          radio_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
          g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_radio_changed), applet);
        }
      else if (type == MENU_ITEM_TYPE_CHECK)
        {
          item = GTK_MENU_ITEM(gtk_check_menu_item_new_with_label(label));
          g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_check_changed), applet);
        }
      else if (type == MENU_ITEM_TYPE_ACTION)
        {
          item = GTK_MENU_ITEM(gtk_menu_item_new_with_label(label));
          g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_menu_command), applet);
        }

      if (item != NULL)
        {
          menu_items_add(applet, id, item);
          if (menu != NULL && type != MENU_ITEM_TYPE_SUBMENU_BEGIN)
            {
              gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(item));
            }
          else if (type != MENU_ITEM_TYPE_SUBMENU_END)
            {
              xfce_panel_plugin_menu_insert_item(applet->plugin, item);
            }

          applet->inhibit++;
          if (GTK_IS_CHECK_MENU_ITEM(item))
            {
              gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), flags & MENU_ITEM_FLAG_ACTIVE);
            }
          applet->inhibit--;
          if (flags & MENU_ITEM_FLAG_VISIBLE)
            {
              gtk_widget_show(GTK_WIDGET(item));
            }
        }

      g_free(label);
    }

  g_variant_iter_free(iter);
}

static void
on_menu_item_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet *)user_data;

  char *text;
  char *dynamic_text;
  char *action;
  uint32_t id;
  uint8_t type;
  uint8_t flags;

  g_variant_get(parameters, "((sssuyy))", &text, &dynamic_text, &action, &id, &type, &flags);

  gchar *label;
  gchar shortcut;
  workrave_extract_shortcut(text, &label, &shortcut);

  GtkMenuItem *item = menu_items_lookup_menu_item_by_id(applet, id);
  if (item != NULL)
    {
      update_menu_item(applet, item, label, flags);
    }

  g_free(text);
  g_free(label);
  g_free(action);
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
  WorkraveApplet *applet = (WorkraveApplet *)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  uint32_t id = 0;
  gboolean valid = menu_items_lookup_id_by_menu_item(applet, item, &id);
  if (!valid)
    {
      return;
    }

  if (id == MENU_COMMAND_ABOUT)
    {
      on_menu_about(item, applet);
    }
  else
    {
      GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->timerbox_control);
      if (proxy != NULL)
        {
          g_dbus_proxy_call(proxy,
                            "Command",
                            g_variant_new("(i)", id),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            (GAsyncReadyCallback)dbus_call_finish,
                            applet);
        }
    }
}

static void
on_menu_about(GtkMenuItem *item, WorkraveApplet *applet)
{
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL);
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

  gtk_container_set_border_width(GTK_CONTAINER(about), 5);

  gtk_show_about_dialog(NULL,
                        "name",
                        "Workrave",
                        "program-name",
                        "Workrave",
#if defined(WORKRAVE_GIT_VERSION)
                        "version",
                        WORKRAVE_VERSION "\n(" WORKRAVE_GIT_VERSION ")",
#else
                        "version",
                        WORKRAVE_VERSION,
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
on_menu_check_changed(GtkMenuItem *item, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet *)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  uint32_t id = 0;
  gboolean valid = menu_items_lookup_id_by_menu_item(applet, item, &id);
  if (!valid)
    {
      return;
    }

  GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "Command",
                        g_variant_new("(i)", id),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback)dbus_call_finish,
                        applet);
    }
}

static void
on_menu_radio_changed(GtkMenuItem *item, gpointer user_data)
{
  WorkraveApplet *applet = (WorkraveApplet *)user_data;

  if (applet->inhibit > 0)
    {
      return;
    }

  uint32_t id = 0;
  gboolean valid = menu_items_lookup_id_by_menu_item(applet, item, &id);
  if (!valid)
    {
      return;
    }

  GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "Command",
                        g_variant_new("(i)", id),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback)dbus_call_finish,
                        applet);
    }
}

static void
workrave_applet_fill(WorkraveApplet *applet)
{
  applet->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->image = workrave_timerbox_control_get_image(applet->timerbox_control);
  g_signal_connect(G_OBJECT(applet->timerbox_control), "alive-changed", G_CALLBACK(on_alive_changed), applet);
  g_signal_connect(G_OBJECT(applet->timerbox_control), "menu-changed", G_CALLBACK(on_menu_changed), applet);
  g_signal_connect(G_OBJECT(applet->timerbox_control), "menu-item-changed", G_CALLBACK(on_menu_item_changed), applet);

  workrave_timerbox_control_set_tray_icon_visible_when_not_running(applet->timerbox_control, TRUE);
  workrave_timerbox_control_set_tray_icon_mode(applet->timerbox_control, WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_FOLLOW);

  gtk_container_add(GTK_CONTAINER(applet->plugin), GTK_WIDGET(applet->image));
  gtk_widget_show_all(GTK_WIDGET(applet->plugin));
}

static void
workrave_applet_free(XfcePanelPlugin *plugin, WorkraveApplet *applet)
{
  g_clear_pointer(&applet->timerbox_control, g_object_unref);
  g_clear_pointer(&applet->menus, g_hash_table_destroy);

  g_slice_free(WorkraveApplet, applet);
}

static void
workrave_applet_construct(XfcePanelPlugin *plugin)
{
  WorkraveApplet *applet = g_slice_new0(WorkraveApplet);
  applet->plugin = plugin;
  applet->image = NULL;
  applet->timerbox_control = NULL;
  applet->alive = FALSE;
  applet->inhibit = 0;

  applet->menus = g_hash_table_new_full(g_int64_hash,
                                        g_int64_equal,
                                        (GDestroyNotify)g_free,
                                        NULL); // (GDestroyNotify)g_object_unref);

  workrave_applet_fill(applet);
  xfce_panel_plugin_set_expand(plugin, FALSE);
  gtk_widget_show_all(GTK_WIDGET(plugin));

  g_signal_connect(plugin, "free-data", G_CALLBACK(workrave_applet_free), applet);
}

XFCE_PANEL_PLUGIN_REGISTER(workrave_applet_construct);
