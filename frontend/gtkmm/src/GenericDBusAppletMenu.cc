// GenericDBusAppletMenu.cc --- Menus using IndicatorApplet+
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

#include "GenericDBusAppletMenu.hh"
#include "GenericDBusAppletWindow.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "Util.hh"

using namespace std;


//! Constructor.
GenericDBusAppletMenu::GenericDBusAppletMenu(GenericDBusAppletWindow *applet_window)
  :applet_window(applet_window)
{
}

//! Destructor.
GenericDBusAppletMenu::~GenericDBusAppletMenu()
{
}

void
GenericDBusAppletMenu::init()
{
}

void
GenericDBusAppletMenu::add_accel(Gtk::Window &window)
{
  (void) window;
}

void
GenericDBusAppletMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  (void) activate_time;
}

void
GenericDBusAppletMenu::add_menu_item(const char *text, int command, int flags)
{
  MenuItem item;
  item.text = text;
  item.command = command;
  item.flags = flags;
  item.group = 0;

  items.push_back(item);
}

void
GenericDBusAppletMenu::resync(OperationMode mode, UsageMode usage, bool show_log)
{
  TRACE_ENTER("GenericDBusAppletMenu::resync");

  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);

      if (iface != NULL)
        {
          iface->Update(WORKRAVE_INDICATOR_SERVICE_OBJ,
                        data[BREAK_ID_MICRO_BREAK], data[BREAK_ID_REST_BREAK], data[BREAK_ID_DAILY_LIMIT]);
        }
    }

  TRACE_EXIT();
  
  if (applet_window != NULL)
    {
      items.clear();
  
      add_menu_item(_("_Open"),        Menus::MENU_COMMAND_OPEN,              MENU_ITEM_FLAG_NONE);
      add_menu_item(_("Preferences"),  Menus::MENU_COMMAND_PREFERENCES,       MENU_ITEM_FLAG_NONE);
      add_menu_item(_("_Rest break"),  Menus::MENU_COMMAND_REST_BREAK,        MENU_ITEM_FLAG_NONE);
      add_menu_item(_("Exercises"),    Menus::MENU_COMMAND_EXERCISES,         MENU_ITEM_FLAG_NONE);
      add_menu_item(_("_Mode"),        0,                                     MENU_ITEM_FLAG_SUBMENU_BEGIN);

      add_menu_item(_("_Normal"),      Menus::MENU_COMMAND_MODE_NORMAL,       MENU_ITEM_FLAG_CHECK
                    | (mode == OPERATION_MODE_NORMAL ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));
      add_menu_item(_("_Suspended"),   Menus::MENU_COMMAND_MODE_SUSPENDED,    MENU_ITEM_FLAG_CHECK
                    | (mode == OPERATION_MODE_SUSPENDED ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));
      add_menu_item(_("Q_uiet"),       Menus::MENU_COMMAND_MODE_QUIET,        MENU_ITEM_FLAG_CHECK
                    | (mode == OPERATION_MODE_QUIET ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));


      add_menu_item(_("_Mode"),        0,                                     MENU_ITEM_FLAG_SUBMENU_END);
 
#ifdef HAVE_DISTRIBUTION
      add_menu_item(_("_Network"),     0,                                     MENU_ITEM_FLAG_SUBMENU_BEGIN);
      add_menu_item(_("_Connect"),    Menus::MENU_COMMAND_NETWORK_CONNECT,    MENU_ITEM_FLAG_NONE);
      add_menu_item(_("_Disconnect"), Menus::MENU_COMMAND_NETWORK_DISCONNECT, MENU_ITEM_FLAG_NONE);
      add_menu_item(_("_Reconnect"),  Menus::MENU_COMMAND_NETWORK_RECONNECT,  MENU_ITEM_FLAG_NONE);
      add_menu_item(_("Show _log"),   Menus::MENU_COMMAND_NETWORK_LOG,        MENU_ITEM_FLAG_CHECK
                    | (show_log ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));

      add_menu_item(_("_Network"),    0,                                      MENU_ITEM_FLAG_SUBMENU_END);
      
#endif
      add_menu_item(_("Reading mode"), Menus::MENU_COMMAND_MODE_READING,      MENU_ITEM_FLAG_CHECK
                    | (usage == USAGE_MODE_READING ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));


      add_menu_item(_("Statistics"),   Menus::MENU_COMMAND_STATISTICS,        MENU_ITEM_FLAG_NONE);
      add_menu_item(_("About..."),     Menus::MENU_COMMAND_ABOUT,             MENU_ITEM_FLAG_NONE);

      applet_window->update_menus(items);
    }
}

