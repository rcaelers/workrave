// Copyright (C) 2002 - 2021 Rob Caelers & Raymond Penners
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
#include "commonui/MenuDefs.hh"
#include "commonui/credits.h"
#include "commonui/nls.h"

#include <glib-object.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

struct _WorkraveAppletPrivate
{
  WorkraveTimerboxControl *timerbox_control;
  GHashTable *radio_actions;
  GActionEntry *action_entries;
  GtkImage *image;
  gboolean alive;
};

G_DEFINE_TYPE_WITH_PRIVATE(WorkraveApplet, workrave_applet, GP_TYPE_APPLET)

static void on_menu_about(GSimpleAction *gaction, GVariant *parameters, gpointer user_data);
static void on_menu_open(GSimpleAction *gaction, GVariant *parameters, gpointer user_data);

static const GActionEntry inactive_menu_actions[] = {{"open", on_menu_open}, {"about", on_menu_about}, {NULL}};

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
send_action(WorkraveApplet *applet, const gchar *action)
{
  GDBusProxy *proxy = workrave_timerbox_control_get_applet_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy,
                        "MenuAction",
                        g_variant_new("(s)", action),
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        (GAsyncReadyCallback)dbus_call_finish,
                        applet);
    }
}

static void
on_menu_action(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const gchar *name = g_action_get_name(G_ACTION(action));
  send_action(applet, name);
}

static void
on_menu_radio_action(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const gchar *mode = g_variant_get_string(value, 0);
  send_action(applet, mode);
}

static void
on_menu_check_action(GSimpleAction *action, GVariant *value, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  const gchar *name = g_action_get_name(G_ACTION(action));
  send_action(applet, name);
}

static void
cleanup_menus(WorkraveApplet *applet)
{
  if (applet->priv->action_entries != NULL)
    {
      for (int i = 0; applet->priv->action_entries[i].name != NULL; i++)
        {
          const GActionEntry *entry = &applet->priv->action_entries[i];

          if (entry->name != NULL)
            {
              g_free((void *)entry->name);
            }
          if (entry->state != NULL)
            {
              g_free((void *)entry->state);
            }
        }
      g_clear_pointer(&applet->priv->action_entries, g_free);
    }
  if (applet->priv->radio_actions != NULL)
    {
      g_hash_table_remove_all(applet->priv->radio_actions);
    }
}

static void
build_menu(WorkraveApplet *applet, GVariant *parameters)
{
  GVariantIter *iter;
  g_variant_get(parameters, "(a(sssuyy))", &iter);

  gchar *text;
  gchar *dynamic_text;
  gchar *action;
  uint32_t id;
  uint8_t type;
  uint8_t flags;

  gchar *group = NULL;
  gchar *active = NULL;
  const gchar *appletid = "workrave";
  gchar *menu = g_strdup_printf("<interface>\n<menu id=\"workrave-menu\">\n<section>\n");

  cleanup_menus(applet);
  gsize num_entries = g_variant_iter_n_children(iter);
  applet->priv->action_entries = g_new0(GActionEntry, num_entries + 1);

  int count = 0;
  while (g_variant_iter_loop(iter, "(sssuyy)", &text, &dynamic_text, &action, &id, &type, &flags))
    {
      char *add = NULL;
      if (type == MENU_ITEM_TYPE_SUBMENU_BEGIN)
        {
          add = g_strdup_printf("<submenu>\n<attribute name=\"label\" translatable=\"yes\">%s</attribute>\n<section>\n", text);
        }
      else if (type == MENU_ITEM_TYPE_SUBMENU_END)
        {
          add = g_strdup_printf("</section>\n</submenu>\n");
        }
      else if (type == MENU_ITEM_TYPE_RADIOGROUP_BEGIN)
        {
          group = g_strdup(action);
        }
      else if (type == MENU_ITEM_TYPE_RADIOGROUP_END)
        {
          applet->priv->action_entries[count].name = group;
          applet->priv->action_entries[count].activate = on_menu_radio_action;
          applet->priv->action_entries[count].parameter_type = "s";
          applet->priv->action_entries[count].state = active;
          applet->priv->action_entries[count].change_state = NULL;
          count++;
          active = NULL;
        }
      else if (type == MENU_ITEM_TYPE_RADIO)
        {
          add = g_strdup_printf(
            "<item>\n<attribute name=\"label\" translatable=\"yes\">%s</attribute>\n<attribute name=\"action\">%s.%s</attribute>\n<attribute name=\"target\">%s</attribute>\n</item>\n",
            text,
            appletid,
            group,
            action);
          if (flags & MENU_ITEM_FLAG_ACTIVE && !active)
            {
              active = g_strdup_printf("'%s'", action);
            }
          g_hash_table_insert(applet->priv->radio_actions, g_strdup(action), g_strdup(group));
        }
      else if (type == MENU_ITEM_TYPE_CHECK)
        {
          add = g_strdup_printf(
            "<item>\n<attribute name=\"label\" translatable=\"yes\">%s</attribute>\n<attribute name=\"action\">%s.%s</attribute>\n</item>\n",
            text,
            appletid,
            action);
          applet->priv->action_entries[count].name = g_strdup(action);
          applet->priv->action_entries[count].activate = on_menu_check_action;
          applet->priv->action_entries[count].parameter_type = NULL;
          applet->priv->action_entries[count].state = g_strdup((flags & MENU_ITEM_FLAG_ACTIVE) ? "true" : "false");
          applet->priv->action_entries[count].change_state = NULL;
          count++;
        }
      else if (type == MENU_ITEM_TYPE_ACTION)
        {
          add = g_strdup_printf(
            "<item>\n<attribute name=\"label\" translatable=\"yes\">%s</attribute>\n<attribute name=\"action\">%s.%s</attribute>\n</item>\n",
            text,
            appletid,
            action);
          applet->priv->action_entries[count].name = g_strdup(action);
          applet->priv->action_entries[count].activate = on_menu_action;
          applet->priv->action_entries[count].parameter_type = NULL;
          applet->priv->action_entries[count].state = NULL;
          applet->priv->action_entries[count].change_state = NULL;
          count++;
        }
      else if (type == MENU_ITEM_TYPE_SEPARATOR)
        {
          add = g_strdup_printf("</section>\n<section>\n");
        }
      if (add != NULL)
        {
          gchar *tmp = g_strconcat(menu, add, NULL);
          g_free(add);
          g_free(menu);
          menu = tmp;
        }
    }

  g_variant_iter_free(iter);

  gchar *add = g_strdup_printf("</section>\n</menu>\n</interface>\n");
  gchar *tmp = g_strconcat(menu, add, NULL);
  g_free(add);
  g_free(menu);
  menu = tmp;

  gp_applet_setup_menu(GP_APPLET(applet), menu, applet->priv->action_entries);
  g_free(menu);
}

static void
on_menu_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);
  build_menu(applet, parameters);
}

static void
on_menu_item_changed(gpointer instance, GVariant *parameters, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  char *text;
  char *dynamic_text;
  char *action;
  uint32_t id;
  uint8_t type;
  uint8_t flags;

  g_variant_get(parameters, "((sssuyy))", &text, &dynamic_text, &action, &id, &type, &flags);

  GAction *gaction = NULL;
  gchar *group = g_hash_table_lookup(applet->priv->radio_actions, action);
  if (group)
    {
      gaction = gp_applet_menu_lookup_action(GP_APPLET(applet), group);
    }
  else
    {
      gaction = gp_applet_menu_lookup_action(GP_APPLET(applet), action);
    }

  if (g_action_get_state_type(G_ACTION(gaction)) != NULL)
    {
      if (type == MENU_ITEM_TYPE_CHECK)
        {
          g_simple_action_set_state(G_SIMPLE_ACTION(gaction), g_variant_new_boolean(flags & MENU_ITEM_FLAG_ACTIVE));
        }
      else if (type == MENU_ITEM_TYPE_RADIO && (flags & MENU_ITEM_FLAG_ACTIVE))
        {
          g_simple_action_set_state(G_SIMPLE_ACTION(gaction), g_variant_new_string(action));
        }
    }

  g_simple_action_set_enabled(G_SIMPLE_ACTION(gaction), (flags & MENU_ITEM_FLAG_VISIBLE));
  g_free(text);
  g_free(action);
}

static void
on_alive_changed(gpointer instance, gboolean alive, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  applet->priv->alive = alive;

  if (alive)
    {
      GVariant *menus = workrave_timerbox_control_get_menus(applet->priv->timerbox_control);
      build_menu(applet, menus);
      g_variant_unref(menus);
    }
  else
    {
      gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-gnome-applet-menu.xml", NULL);
      gp_applet_setup_menu_from_file(GP_APPLET(applet), ui_path, inactive_menu_actions);
      g_free(ui_path);
    }
}

static void
on_menu_open(GSimpleAction *gaction, GVariant *parameter, gpointer user_data)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(user_data);

  GDBusProxy *proxy = workrave_timerbox_control_get_control_proxy(applet->priv->timerbox_control);
  if (proxy != NULL)
    {
      g_dbus_proxy_call(proxy, "OpenMain", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, (GAsyncReadyCallback)dbus_call_finish, applet);
    }
}

static void
on_menu_about(GSimpleAction *gaction, GVariant *parameter, gpointer user_data)
{
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(WORKRAVE_PKGDATADIR "/images/workrave.png", NULL);
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

  gtk_container_set_border_width(GTK_CONTAINER(about), 5);

  gtk_show_about_dialog(NULL,
                        "name",
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
workrave_applet_fill(WorkraveApplet *applet)
{
  applet->priv->timerbox_control = g_object_new(WORKRAVE_TIMERBOX_CONTROL_TYPE, NULL);
  applet->priv->image = workrave_timerbox_control_get_image(applet->priv->timerbox_control);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "alive-changed", G_CALLBACK(on_alive_changed), applet);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "menu-changed", G_CALLBACK(on_menu_changed), applet);
  g_signal_connect(G_OBJECT(applet->priv->timerbox_control), "menu-item-changed", G_CALLBACK(on_menu_item_changed), applet);

  workrave_timerbox_control_set_tray_icon_visible_when_not_running(applet->priv->timerbox_control, TRUE);
  workrave_timerbox_control_set_tray_icon_mode(applet->priv->timerbox_control, WORKRAVE_TIMERBOX_CONTROL_TRAY_ICON_MODE_FOLLOW);

  gchar *ui_path = g_build_filename(WORKRAVE_UIDATADIR, "workrave-gnome-applet-menu.xml", NULL);
  gp_applet_setup_menu_from_file(GP_APPLET(applet), ui_path, inactive_menu_actions);
  g_free(ui_path);

  gp_applet_set_flags(GP_APPLET(applet), GP_APPLET_FLAGS_EXPAND_MINOR);

  gtk_container_set_border_width(GTK_CONTAINER(applet), 0);

  gtk_widget_set_events(GTK_WIDGET(applet), gtk_widget_get_events(GTK_WIDGET(applet)) | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(applet), "button_press_event", G_CALLBACK(button_pressed), applet);

  gtk_container_add(GTK_CONTAINER(applet), GTK_WIDGET(applet->priv->image));

  gtk_widget_show(GTK_WIDGET(applet->priv->image));
  gtk_widget_show(GTK_WIDGET(applet));

  on_alive_changed(NULL, FALSE, applet);
}

static void
workrave_applet_constructed(GObject *object)
{
  G_OBJECT_CLASS(workrave_applet_parent_class)->constructed(object);
  workrave_applet_fill(WORKRAVE_APPLET(object));
}

static void
workrave_applet_dispose(GObject *object)
{
  WorkraveApplet *applet = WORKRAVE_APPLET(object);
  g_clear_pointer(&applet->priv->timerbox_control, g_object_unref);
  g_clear_pointer(&applet->priv->radio_actions, g_hash_table_unref);
  cleanup_menus(applet);
  G_OBJECT_CLASS(workrave_applet_parent_class)->dispose(object);
}

static void
workrave_applet_class_init(WorkraveAppletClass *class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS(class);
  object_class->constructed = workrave_applet_constructed;
  object_class->dispose = workrave_applet_dispose;
}

static void
workrave_applet_init(WorkraveApplet *applet)
{
  applet->priv = workrave_applet_get_instance_private(applet);

  applet->priv->image = NULL;
  applet->priv->timerbox_control = NULL;
  applet->priv->alive = FALSE;
  applet->priv->radio_actions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
}
