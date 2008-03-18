// MainGtkMenu.cc --- Menus using Gtk+
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

static const char rcsid[] = "$Id: MainGtkMenu.cc 1436 2008-02-03 18:03:23Z rcaelers $";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "MainGtkMenu.hh"

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


//! Constructor.
MainGtkMenu::MainGtkMenu(bool show_open)
  : popup_menu(NULL),
    show_open(show_open)
{
}


//! Destructor.
MainGtkMenu::~MainGtkMenu()
{
}


void
MainGtkMenu::add_stock_item(const Glib::RefPtr<Gtk::IconFactory>& factory,
                        const std::string &path,
                        const Glib::ustring& icon_id,
                        const Glib::ustring& label)
{
  Gtk::IconSource source;
  Gtk::IconSet icon_set;

  string filename = Util::complete_directory(path, Util::SEARCH_PATH_IMAGES);

  try
  {
    source.set_pixbuf(Gdk::Pixbuf::create_from_file(filename));
  }
  catch(const Glib::Exception& ex)
  {
  }

  source.set_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
  source.set_size_wildcarded();

  icon_set.add_source(source);

  const Gtk::StockID stock_id(icon_id);
  factory->add(stock_id, icon_set);
  Gtk::Stock::add(Gtk::StockItem(stock_id, label));
}


void
MainGtkMenu::register_stock_items()
{
  Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();

  add_stock_item(factory, "timer-rest-break.png", "restbreak", _("_Rest break"));

  factory->add_default();
}


void
MainGtkMenu::init()
{
  if (popup_menu == NULL)
    {
      register_stock_items();
      create_actions();
      create_ui();
      post_init();
    }
  else
    {
      // Re-init.
      ui_manager->remove_action_group(action_group);
      create_actions();
      ui_manager->insert_action_group(action_group);
    }
}

void
MainGtkMenu::add_accel(Gtk::Window &window)
{
  window.add_accel_group(ui_manager->get_accel_group());
}


void
MainGtkMenu::create_actions()
{
  Menus *menus = Menus::get_instance();

  action_group = Gtk::ActionGroup::create();

  action_group->add(Gtk::Action::create("Main", _("_Tools")));
  
  // Mode menu
  Gtk::RadioAction::Group group_mode;
  action_group->add(Gtk::Action::create("Mode", _("_Mode")));

  action_group->add(Gtk::RadioAction::create(group_mode, "Normal", _("_Normal")),
                    sigc::mem_fun(*this, &MainGtkMenu::on_menu_normal));
  
  action_group->add(Gtk::RadioAction::create(group_mode, "Suspended", _("_Suspended")),
                    sigc::mem_fun(*this, &MainGtkMenu::on_menu_suspend));

  action_group->add(Gtk::RadioAction::create(group_mode, "Quiet", _("Q_uiet")),
                    sigc::mem_fun(*this, &MainGtkMenu::on_menu_quiet));
  
  // Networking menu
  action_group->add(Gtk::Action::create("Network", _("_Network")));
  action_group->add(Gtk::Action::create("Join", _("_Connect")),
                    sigc::mem_fun(*menus, &Menus::on_menu_network_join));
  action_group->add(Gtk::Action::create("Disconnect", _("_Disconnect")),
                    sigc::mem_fun(*menus, &Menus::on_menu_network_leave));
  action_group->add(Gtk::Action::create("Reconnect", _("_Reconnect")),
                    sigc::mem_fun(*menus, &Menus::on_menu_network_reconnect));
  action_group->add(Gtk::ToggleAction::create("ShowLog", _("Show _log")),
                    sigc::mem_fun(*this, &MainGtkMenu::on_menu_network_log));
  
  // Open
  action_group->add(Gtk::Action::create("Open", Gtk::Stock::OPEN),
                    sigc::mem_fun(*menus, &Menus::on_menu_open_main_window));

  // Restbreak now
  // Gtk::AccelKey("<control>r"),
  action_group->add(Gtk::Action::create("Restbreak",
                                        Gtk::StockID("restbreak"),
                                        _("_Rest break")),
                    sigc::mem_fun(*menus, &Menus::on_menu_restbreak_now));
  
  // Preferences
  action_group->add(Gtk::Action::create("Preferences",
                                        Gtk::Stock::PREFERENCES),
                    sigc::mem_fun(*menus, &Menus::on_menu_preferences));

  action_group->add(Gtk::Action::create("Exercises",
                                        _("Exercises")),
                    sigc::mem_fun(*menus, &Menus::on_menu_exercises));

  action_group->add(Gtk::Action::create("Statistics",
                                        _("Statistics")),
                    sigc::mem_fun(*menus, &Menus::on_menu_statistics));

  action_group->add(Gtk::Action::create("About",
                                        Gtk::Stock::ABOUT),
                    sigc::mem_fun(*menus, &Menus::on_menu_about));
  
  action_group->add(Gtk::Action::create("Quit",
                                        Gtk::Stock::QUIT),
                    sigc::mem_fun(*menus, &Menus::on_menu_quit));
}

void
MainGtkMenu::create_ui()
{
  Glib::ustring open_ui_info;

  if (show_open)
    {
      open_ui_info = "    <menuitem action='Open'/>";
    }

  //Layout the actions in a menubar and toolbar:
  Glib::ustring ui_info =
    "<ui>"
    "  <popup name='Menu'>"
    + open_ui_info +
    "    <separator/>" +
    "    <menuitem action='Restbreak'/>"
    "    <menuitem action='Exercises'/>"
    "    <menuitem action='Statistics'/>"
    "    <separator/>" +
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
    "    <separator/>" +
    "    <menuitem action='Preferences'/>"
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
    }

  popup_menu = dynamic_cast<Gtk::Menu*>(ui_manager->get_widget("/Menu")); 
}


void
MainGtkMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  
  if (popup_menu != NULL)
    {
      popup_menu->popup(button, activate_time);
    }
}


void
MainGtkMenu::resync(OperationMode mode, bool show_log)
{
  Gtk::CheckMenuItem *item = NULL;
  const char *menu_name = NULL;
  
  switch (mode)
    {
    case OPERATION_MODE_NORMAL:
      menu_name = "/Menu/Mode/Normal";
      break;
        
    case OPERATION_MODE_SUSPENDED:
      menu_name = "/Menu/Mode/Suspended";
      break;

    case OPERATION_MODE_QUIET:
      menu_name = "/Menu/Mode/Quiet";
      break;

    default:
      break;
    }

  if (menu_name != NULL)
    {
      item = dynamic_cast<Gtk::CheckMenuItem*>(ui_manager->get_widget(menu_name));
      if (item != NULL)
        {
          item->set_active(true);
        }
    }

  item = dynamic_cast<Gtk::CheckMenuItem*>(ui_manager->get_widget("/Menu/Network/ShowLog"));
  if (item != NULL)
    {
      item->set_active(show_log);
    }
}


void
MainGtkMenu::on_menu_network_log()
{
  Glib::RefPtr<Gtk::Action> act = ui_manager->get_action("/Menu/Network/ShowLog");
  Glib::RefPtr<Gtk::ToggleAction> ract = Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(act);

  if (ract)
    {
      bool active = ract->get_active();
      Menus *menus = Menus::get_instance();
      menus->on_menu_network_log(active);
    }
}

void
MainGtkMenu::on_menu_normal()
{
  Glib::RefPtr<Gtk::Action> act = ui_manager->get_action("/Menu/Mode/Normal");
  Glib::RefPtr<Gtk::RadioAction> ract = Glib::RefPtr<Gtk::RadioAction>::cast_dynamic(act);

  if (ract)
    {
      bool active = ract->get_active();
      if (active)
        {
          Menus *menus = Menus::get_instance();
          menus->on_menu_normal();
        }
    }
}

void
MainGtkMenu::on_menu_suspend()
{
  Glib::RefPtr<Gtk::Action> act = ui_manager->get_action("/Menu/Mode/Suspended");
  Glib::RefPtr<Gtk::RadioAction> ract = Glib::RefPtr<Gtk::RadioAction>::cast_dynamic(act);

  if (ract)
    {
      bool active = ract->get_active();
      if (active)
        {
          Menus *menus = Menus::get_instance();
          menus->on_menu_suspend();
        }
    }
}

void
MainGtkMenu::on_menu_quiet()
{
  Glib::RefPtr<Gtk::Action> act = ui_manager->get_action("/Menu/Mode/Quiet");
  Glib::RefPtr<Gtk::RadioAction> ract = Glib::RefPtr<Gtk::RadioAction>::cast_dynamic(act);

  if (ract)
    {
      bool active = ract->get_active();
      if (active)
        {
          Menus *menus = Menus::get_instance();
          menus->on_menu_quiet();
        }
    }
}
