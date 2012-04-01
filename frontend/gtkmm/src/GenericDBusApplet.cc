// GenericDBusApplet.cc --- Applet info Window
//
// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#include <gtkmm.h>

#include "GenericDBusApplet.hh"

#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"
#include "CoreFactory.hh"

#include "DBus.hh"
#include "DBusException.hh"
#include "DBusGUI.hh"

#define  WORKRAVE_INDICATOR_SERVICE_NAME     "org.workrave.Workrave"
#define  WORKRAVE_INDICATOR_SERVICE_IFACE    "org.workrave.AppletInterface"
#define  WORKRAVE_INDICATOR_SERVICE_OBJ      "/org/workrave/Workrave/UI"

//! Constructor.
GenericDBusApplet::GenericDBusApplet() :
  enabled(false), dbus(NULL)
{
  timer_box_control = new TimerBoxControl("applet", *this);
  timer_box_view = this;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      data[i].bar_text = "";
      data[i].bar_primary_color = 0;
      data[i].bar_primary_val = 0;
      data[i].bar_primary_max = 0;
      data[i].bar_secondary_color = 0;
      data[i].bar_secondary_val = 0;
      data[i].bar_secondary_max = 0;
    }
}


//! Destructor.
GenericDBusApplet::~GenericDBusApplet()
{
}

void
GenericDBusApplet::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("GenericDBusApplet::set_slot", int(id) << ", " << slot);
  data[slot].slot = id;
  TRACE_EXIT();
}

void
GenericDBusApplet::set_time_bar(BreakId id,
                                std::string text,
                                ITimeBar::ColorId primary_color,
                                int primary_val, int primary_max,
                                ITimeBar::ColorId secondary_color,
                                int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("GenericDBusApplet::set_time_bar", int(id) << "=" << text);
  data[id].bar_text = text;
  data[id].bar_primary_color = primary_color;
  data[id].bar_primary_val = primary_val;
  data[id].bar_primary_max = primary_max;
  data[id].bar_secondary_color = secondary_color;
  data[id].bar_secondary_val = secondary_val;
  data[id].bar_secondary_max = secondary_max;
  TRACE_EXIT();
}

void
GenericDBusApplet::update_view()
{
  TRACE_ENTER("GenericDBusApplet::update_view");

  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  assert(iface != NULL);
  iface->TimersUpdated(WORKRAVE_INDICATOR_SERVICE_OBJ,
                       data[BREAK_ID_MICRO_BREAK], data[BREAK_ID_REST_BREAK], data[BREAK_ID_DAILY_LIMIT]);

  TRACE_EXIT();
}

void
GenericDBusApplet::init_applet()
{
  try
    {
      dbus = CoreFactory::get_dbus();
      if (dbus != NULL && dbus->is_available())
        {
          dbus->connect(WORKRAVE_INDICATOR_SERVICE_OBJ,
                        WORKRAVE_INDICATOR_SERVICE_IFACE,
                        this);
        }
    }
  catch (DBusException)
    {
    }
}

//! Initializes the applet.
AppletWindow::AppletState
GenericDBusApplet::activate_applet()
{
  TRACE_ENTER("GenericDBusApplet::activate_applet");
  TRACE_EXIT();
  return enabled ? AppletWindow::APPLET_STATE_VISIBLE : AppletWindow::APPLET_STATE_PENDING;
}


//! Destroys the applet.
void
GenericDBusApplet::deactivate_applet()
{
  TRACE_ENTER("GenericDBusApplet::deactivate_applet");
  enabled = false;
  state_changed_signal.emit(AppletWindow::APPLET_STATE_DISABLED);

  for (std::set<std::string>::iterator i = active_bus_names.begin(); i != active_bus_names.end(); i++)
    {
      dbus->unwatch(*i);
    }
  active_bus_names.clear();
  TRACE_EXIT();
}

//! 
void
GenericDBusApplet::applet_embed(bool enable, const string &sender)
{
  TRACE_ENTER_MSG("GenericDBusApplet::applet_embed", enable << " " << sender);
  if (sender != "")
    {
      dbus->watch(sender, this);
    }
  // else... FIXME:
  TRACE_EXIT();
}

void
GenericDBusApplet::resync(OperationMode mode, UsageMode usage, bool show_log)
{
  TRACE_ENTER("GenericDBusAppletMenu::resync");

  items.clear();
  
  add_menu_item(_("Open"),        Menus::MENU_COMMAND_OPEN,              MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Preferences"), Menus::MENU_COMMAND_PREFERENCES,       MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Rest break"),  Menus::MENU_COMMAND_REST_BREAK,        MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Exercises"),   Menus::MENU_COMMAND_EXERCISES,         MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Mode"),        0,                                     MENU_ITEM_FLAG_SUBMENU_BEGIN);

  add_menu_item(_("Normal"),      Menus::MENU_COMMAND_MODE_NORMAL,       MENU_ITEM_FLAG_RADIO
                | (mode == OPERATION_MODE_NORMAL ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));
  add_menu_item(_("Suspended"),   Menus::MENU_COMMAND_MODE_SUSPENDED,    MENU_ITEM_FLAG_RADIO
                | (mode == OPERATION_MODE_SUSPENDED ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));
  add_menu_item(_("Quiet"),       Menus::MENU_COMMAND_MODE_QUIET,        MENU_ITEM_FLAG_RADIO
                | (mode == OPERATION_MODE_QUIET ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));


  add_menu_item(_("Mode"),        0,                                     MENU_ITEM_FLAG_SUBMENU_END);
 
#ifdef HAVE_DISTRIBUTION
  add_menu_item(_("Network"),     0,                                     MENU_ITEM_FLAG_SUBMENU_BEGIN);
  add_menu_item(_("Connect"),    Menus::MENU_COMMAND_NETWORK_CONNECT,    MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Disconnect"), Menus::MENU_COMMAND_NETWORK_DISCONNECT, MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Reconnect"),  Menus::MENU_COMMAND_NETWORK_RECONNECT,  MENU_ITEM_FLAG_NONE);
  add_menu_item(_("Show log"),   Menus::MENU_COMMAND_NETWORK_LOG,        MENU_ITEM_FLAG_CHECK
                | (show_log ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));

  add_menu_item(_("Network"),    0,                                      MENU_ITEM_FLAG_SUBMENU_END);
      
#endif
  add_menu_item(_("Reading mode"), Menus::MENU_COMMAND_MODE_READING,      MENU_ITEM_FLAG_CHECK
                | (usage == USAGE_MODE_READING ? MENU_ITEM_FLAG_ACTIVE : MENU_ITEM_FLAG_NONE));


  add_menu_item(_("Statistics"),   Menus::MENU_COMMAND_STATISTICS,        MENU_ITEM_FLAG_NONE);
  add_menu_item(_("About..."),     Menus::MENU_COMMAND_ABOUT,             MENU_ITEM_FLAG_NONE);


  org_workrave_AppletInterface *iface = org_workrave_AppletInterface::instance(dbus);
  assert(iface != NULL);
  iface->MenuUpdated(WORKRAVE_INDICATOR_SERVICE_OBJ, items);
  
  TRACE_EXIT();
}

void
GenericDBusApplet::get_menu(MenuItems &out) const
{
  out = items;
}

void
GenericDBusApplet::add_menu_item(const char *text, int command, int flags)
{
  MenuItem item;
  item.text = text;
  item.command = command;
  item.flags = flags;
  items.push_back(item);
}

void
GenericDBusApplet::applet_command(int command)
{
  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  menus->applet_command(command);
}

void
GenericDBusApplet::bus_name_presence(const std::string &name, bool present)
{
  TRACE_ENTER_MSG("GenericDBusApplet::bus_name_presence", name << " " << present);
  if (present)
    {
      active_bus_names.insert(name);
      if (!enabled)
        {
          TRACE_MSG("Enabling");
          enabled = true;
          state_changed_signal.emit(AppletWindow::APPLET_STATE_VISIBLE);
        }
    }
  else
    {
      active_bus_names.erase(name);
      if (active_bus_names.size() == 0)
        {
          TRACE_MSG("Disabling");
          state_changed_signal.emit(AppletWindow::APPLET_STATE_DISABLED);
          enabled = false;
        }
      // TODO: unwatch or not? dbus->unwatch(name);
    }
  TRACE_EXIT();
}
