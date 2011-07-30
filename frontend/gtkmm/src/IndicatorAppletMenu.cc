// IndicatorAppletMenu.cc --- Menus using IndicatorApplet+
//
// Copyright (C) 2011 Rob Caelers <robc@krandor.nl>
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <string>

#include <libindicator/indicator-service.h>

#include "IndicatorAppletMenu.hh"
#include "indicator_applet/dbus-shared.h"
#include "IndicatorAppletWindow.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "Util.hh"

using namespace std;


//! Constructor.
IndicatorAppletMenu::IndicatorAppletMenu(IndicatorAppletWindow *applet_window)
  :applet_window(applet_window)
{
}

//! Destructor.
IndicatorAppletMenu::~IndicatorAppletMenu()
{
  // FIXME: cleanup.
}

DbusmenuMenuitem *
IndicatorAppletMenu::menu_item_append(DbusmenuMenuitem *parent, const char *label)
{
  return menu_item_append(parent, label, Normal, -1);
}

DbusmenuMenuitem *
IndicatorAppletMenu::menu_item_append(DbusmenuMenuitem *parent, const char *label, int cmd)
{
  return menu_item_append(parent, label, Normal, cmd);
}

DbusmenuMenuitem *
IndicatorAppletMenu::menu_item_append(DbusmenuMenuitem *parent, const char *label, MenuItemType type, int cmd)
{
  DbusmenuMenuitem *item = dbusmenu_menuitem_new();
  dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_LABEL, label);

  switch (type)
    {
    case Radio:
      dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_RADIO);
      break;
    case Check:
      dbusmenu_menuitem_property_set(item, DBUSMENU_MENUITEM_PROP_TOGGLE_TYPE, DBUSMENU_MENUITEM_TOGGLE_CHECK);
      break;
    case Normal:
      break;
    }

  dbusmenu_menuitem_child_append(parent, item);

  g_signal_connect(G_OBJECT(item), DBUSMENU_MENUITEM_SIGNAL_ITEM_ACTIVATED, G_CALLBACK(static_menu_item_activated), this);

  if (cmd != -1)
    {
      menu_items[cmd] = item;
    }

  return item;
}

void
IndicatorAppletMenu::menu_item_set_checked(int cmd, bool checked)
{
  dbusmenu_menuitem_property_set_int(menu_items[cmd], DBUSMENU_MENUITEM_PROP_TOGGLE_STATE,
                                     checked ?
                                     DBUSMENU_MENUITEM_TOGGLE_STATE_CHECKED :
                                     DBUSMENU_MENUITEM_TOGGLE_STATE_UNCHECKED);
}

void
IndicatorAppletMenu::init()
{
  // FIXME: can be removed once Workrave sed gio dbus
	g_bus_own_name(G_BUS_TYPE_SESSION, WORKRAVE_INDICATOR_MENU_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
	               NULL, NULL, NULL, NULL, NULL);

	server = dbusmenu_server_new(WORKRAVE_INDICATOR_MENU_OBJ);
	root = dbusmenu_menuitem_new();
	dbusmenu_server_set_root(server, root);
  dbusmenu_menuitem_property_set_bool(root, DBUSMENU_MENUITEM_PROP_VISIBLE, TRUE);

  menu_item_append(root, _("Open"), Menus::MENU_COMMAND_OPEN);
  menu_item_append(root, _("Preferences"), Menus::MENU_COMMAND_PREFERENCES);
  menu_item_append(root, _("_Rest break"), Menus::MENU_COMMAND_REST_BREAK);
  menu_item_append(root, _("Exercises"), Menus::MENU_COMMAND_EXERCISES);

  DbusmenuMenuitem *mode_menu = menu_item_append(root, _("_Mode"));

  menu_item_append(mode_menu, _("_Normal"), Radio, Menus::MENU_COMMAND_MODE_NORMAL);
  menu_item_append(mode_menu, _("_Suspended"), Radio, Menus::MENU_COMMAND_MODE_SUSPENDED);
  menu_item_append(mode_menu, _("Q_uiet"), Radio, Menus::MENU_COMMAND_MODE_QUIET);

  DbusmenuMenuitem *network_menu = menu_item_append(root, _("_Network"));

  menu_item_append(network_menu, _("_Connect"), Menus::MENU_COMMAND_NETWORK_CONNECT);
  menu_item_append(network_menu, _("_Disconnect"), Menus::MENU_COMMAND_NETWORK_DISCONNECT);
  menu_item_append(network_menu, _("_Reconnect"), Menus::MENU_COMMAND_NETWORK_RECONNECT);
  menu_item_append(network_menu, _("Show _log"), Check, Menus::MENU_COMMAND_NETWORK_LOG);

  menu_item_append(root, _("Reading mode"), Check, Menus::MENU_COMMAND_MODE_READING);
  menu_item_append(root, _("Statistics"), Menus::MENU_COMMAND_STATISTICS);
  menu_item_append(root, _("About..."), Menus::MENU_COMMAND_ABOUT);
  menu_item_append(root, _("Quit"), Menus::MENU_COMMAND_QUIT);
}

void
IndicatorAppletMenu::add_accel(Gtk::Window &window)
{
  (void) window;
}

void
IndicatorAppletMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  (void) activate_time;
}

void
IndicatorAppletMenu::resync(OperationMode mode, UsageMode usage, bool show_log)
{
  menu_item_set_checked(Menus::MENU_COMMAND_MODE_NORMAL, mode == OPERATION_MODE_NORMAL);
  menu_item_set_checked(Menus::MENU_COMMAND_MODE_QUIET, mode == OPERATION_MODE_QUIET);
  menu_item_set_checked(Menus::MENU_COMMAND_MODE_SUSPENDED, mode == OPERATION_MODE_SUSPENDED);
  menu_item_set_checked(Menus::MENU_COMMAND_MODE_READING, usage == USAGE_MODE_READING);
  menu_item_set_checked(Menus::MENU_COMMAND_NETWORK_LOG, show_log);
}

int
IndicatorAppletMenu::find_menu_item(DbusmenuMenuitem *item) const
{
  for (int i = 0; i < Menus::MENU_COMMAND_SIZEOF; i++)
    {
      if (menu_items[i] == item)
        {
          return i;
        }
    }

  return -1;
}

void
IndicatorAppletMenu::static_menu_item_activated(DbusmenuMenuitem *mi, guint timestamp, gpointer user_data)
{
  (void) timestamp;

  IndicatorAppletMenu *menu = (IndicatorAppletMenu *) user_data;
  menu->menu_item_activated(mi);
}

void
IndicatorAppletMenu::menu_item_activated(DbusmenuMenuitem *mi)
{
  int command = find_menu_item(mi);
  if (command != -1)
    {
      GUI *gui = GUI::get_instance();
      Menus *menus = gui->get_menus();;
      menus->applet_command(command);
    }
}
