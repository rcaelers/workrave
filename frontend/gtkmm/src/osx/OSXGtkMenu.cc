// OSXGtkMenu.cc --- Menus using Gtk+
//
// Copyright (C) 2001 - 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id: OSXGtkMenu.cc 1436 2008-02-03 18:03:23Z rcaelers $";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "OSXGtkMenu.hh"

#include <string>

#include <gdkmm/pixbuf.h>
#include <gtkmm/action.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "Menus.hh"
#include "Util.hh"

using namespace std;

#include "ige-mac-menu.h"
#include "ige-mac-dock.h"
#include "ige-mac-bundle.h"


//! Constructor.
OSXGtkMenu::OSXGtkMenu(bool show_open)
  : popup_menu(NULL),
    show_open(show_open)
{
  register_stock_items();
  create_menu();
}


//! Destructor.
OSXGtkMenu::~OSXGtkMenu()
{
}


void
OSXGtkMenu::dock_clicked(IgeMacDock *dock, void *data)
{
  (void) dock;
  Menus *menus = (Menus *) data;
  menus->on_menu_open_main_window();
}


void
OSXGtkMenu::dock_quit(IgeMacDock *dock, void *data)
{
  (void) dock;
  Menus *menus = (Menus *) data;
  menus->on_menu_quit();
}

void
OSXGtkMenu::create_ui()
{
  //Layout the actions in a menubar and toolbar:
  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='Applet'>"
    "    <menuitem action='Preferences'/>"
    "    <menuitem action='Restbreak'/>"
    "    <menuitem action='Exercises'/>"
    "    <menu action='Mode'>"
    "      <menuitem action='Normal'/>"
    "      <menuitem action='Suspended'/>"
    "      <menuitem action='Quiet'/>"
    "    </menu>"
    "    <menu action='Network'>"
    "      <menuitem action='Join'/>"
    "      <menuitem action='Disconnect'/>"
    "      <menuitem action='Reconnect'/>"
    "      <menuitem action='ShowLog'/>"
    "    </menu>"
    "    <menuitem action='Statistics'/>"
    "    <menuitem action='About'/>"
    "    <menuitem action='Quit'/>"
    "  </popup>"
    "</ui>";

  ui_manager = Gtk::UIManager::create();
  ui_manager->insert_action_group(action_group);
  
  try
    {
      ui_manager->add_ui_from_string(ui_info);
    }
  catch(const Glib::Error& ex)
    {
      std::cerr << "building menus and toolbars failed: " <<  ex.what();
    }

  IgeMacMenuGroup *group;
  IgeMacDock      *dock;

  
  Gtk::Menu *menu = dynamic_cast<Gtk::Menu*>(ui_manager->get_widget("/Applet")); 
  
  ige_mac_menu_set_menu_bar(GTK_MENU_SHELL(pop_menu->gobj()));
  ige_mac_menu_set_quit_menu_item(GTK_MENU_ITEM(quit_item.get_child()->gobj()));
          
  group = ige_mac_menu_add_app_menu_group();
  ige_mac_menu_add_app_menu_item(group,
                                 GTK_MENU_ITEM(about_item.get_child()->gobj()), 
                                 NULL);
          
  group = ige_mac_menu_add_app_menu_group();
  ige_mac_menu_add_app_menu_item(group,
                                 GTK_MENU_ITEM (pref_item.get_child()->gobj()), 
                                 NULL);

  dock = ige_mac_dock_new ();
  g_signal_connect(dock,
                   "clicked",
                   G_CALLBACK(dock_clicked),
                   this);
  g_signal_connect(dock,
                   "quit-activate",
                   G_CALLBACK(dock_quit),
                   this);

}
