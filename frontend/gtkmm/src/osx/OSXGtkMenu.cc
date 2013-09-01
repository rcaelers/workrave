// OSXGtkMenu.cc --- Menus using Gtk+
//
// Copyright (C) 2001 - 2011, 2013 Rob Caelers & Raymond Penners
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

#include <iostream>

#include "nls.h"
#include "debug.hh"

#include "OSXGtkMenu.hh"

#include <string>

#include <gtkmm.h>

#include "Menus.hh"
#include "Util.hh"

using namespace std;

#if HAVE_IGE_MAC_INTEGRATION
#include "ige-mac-menu.h"
#include "ige-mac-dock.h"
#include "ige-mac-bundle.h"
#endif

#if HAVE_GTK_MAC_INTEGRATION
#include "gtk-mac-menu.h"
#include "gtk-mac-dock.h"
#include "gtk-mac-bundle.h"
#define IgeMacMenuGroup GtkMacMenuGroup
#define IgeMacDock GtkMacDock
#define ige_mac_menu_set_menu_bar gtk_mac_menu_set_menu_bar
#define ige_mac_menu_set_quit_menu_item gtk_mac_menu_set_quit_menu_item
#define ige_mac_menu_add_app_menu_group gtk_mac_menu_add_app_menu_group
#define ige_mac_menu_add_app_menu_item gtk_mac_menu_add_app_menu_item
#define ige_mac_dock_new gtk_mac_dock_new
#endif


//! Constructor.
OSXGtkMenu::OSXGtkMenu(bool show_open)
  : MainGtkMenu(show_open)
{
}


//! Destructor.
OSXGtkMenu::~OSXGtkMenu()
{
}


void
OSXGtkMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  (void) activate_time;
}

void
OSXGtkMenu::dock_clicked(IgeMacDock *dock, void *data)
{
  (void) dock;
  // current, segment fault
  // Menus *menus = (Menus *) data;
  // menus->on_menu_open_main_window();
}


void
OSXGtkMenu::dock_quit(IgeMacDock *dock, void *data)
{
  (void) dock;
  // current, segment fault
  // Menus *menus = (Menus *) data;
  // menus->on_menu_quit();
}

void
OSXGtkMenu::create_ui()
{
  Glib::ustring ui_info =
    "<ui>\n"
    "  <menubar name='Apple'>\n"
    "    <menuitem action='Preferences'/>\n"
    "    <menuitem action='About'/>\n"
    "    <menuitem action='Quit'/>\n"
    "  </menubar>\n"
    "  <menubar name='Menu'>\n"
    "    <menu action='Main'>\n"
    "      <menuitem action='Restbreak'/>\n"
    "      <menuitem action='Exercises'/>\n"
    "      <menuitem action='Statistics'/>\n"
    "    </menu>\n"
    "    <menu action='Mode'>\n"
    "      <menuitem action='Normal'/>\n"
    "      <menuitem action='Quiet'/>\n"
    "      <menuitem action='Suspended'/>\n"
    "    </menu>\n"
    "    <menuitem action='Reading mode'/>"
    "  </menubar>\n"
    "</ui>\n";

  // ui_manager = Gtk::UIManager::create();
  // ui_manager->insert_action_group(action_group);

  // try
  //   {
  //     ui_manager->add_ui_from_string(ui_info);
  //   }
  // catch(const Glib::Error& ex)
  //   {
  //     std::cerr << "building menus and toolbars failed: " <<  ex.what();
  //   }


  // Glib::RefPtr<Gtk::Application> app = Gtk::Application::get_default();

  // static const GActionEntry actions[] = {
  //   { "new", new_window },
  //   { "quit", quit }
  // };
  
  // GMenu *menu;

  // g_action_map_add_action_entries (G_ACTION_MAP (application), actions, G_N_ELEMENTS (actions), application);

  // menu = g_menu_new ();
  // g_menu_append (menu, "New", "app.new");
  // g_menu_append (menu, "Quit", "app.quit");
  // gtk_application_set_app_menu (application, G_MENU_MODEL (menu));
  // g_object_unref (menu);  

  // app->set_appmenu()

    
  // Gtk::MenuBar *menu = dynamic_cast<Gtk::MenuBar*>(ui_manager->get_widget("/Menu"));
  // Gtk::MenuItem *item = dynamic_cast<Gtk::MenuItem*>(ui_manager->get_widget("/Apple/Quit"));

  // ige_mac_menu_set_menu_bar(GTK_MENU_SHELL(menu->gobj()));

  // ige_mac_menu_set_quit_menu_item(GTK_MENU_ITEM(item->gobj()));

  // item = dynamic_cast<Gtk::MenuItem*>(ui_manager->get_widget("/Apple/About"));

  // group = ige_mac_menu_add_app_menu_group();
  // ige_mac_menu_add_app_menu_item(group,
  //                                GTK_MENU_ITEM(item->gobj()),
  //                                NULL);

  // item = dynamic_cast<Gtk::MenuItem*>(ui_manager->get_widget("/Apple/Preferences"));

  // group = ige_mac_menu_add_app_menu_group();
  // ige_mac_menu_add_app_menu_item(group,
  //                                GTK_MENU_ITEM (item->gobj()),
  //                                NULL);

  // dock = ige_mac_dock_new ();
  // g_signal_connect(dock,
  //                  "clicked",
  //                  G_CALLBACK(dock_clicked),
  //                  this);

  // g_signal_connect(dock,
  //                  "quit-activate",
  //                  G_CALLBACK(dock_quit),
  //                  this);

}
