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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "commonui/nls.h"
#include "debug.hh"

#include "GUI.hh"
#include "MainGtkMenu.hh"

#include <string>

#include <giomm.h>
#include <gtkmm.h>

#include "MainWindow.hh"
#include "Menus.hh"
#include "utils/AssetPath.hh"

using namespace std;

using namespace workrave;
using namespace workrave::utils;

//! Constructor.
MainGtkMenu::MainGtkMenu(bool show_open)
  : show_open(show_open)
{
}

void
MainGtkMenu::init()
{
  if (popup_menu == nullptr)
    {
      create_actions();
      create_menu();
      post_init();

      // TODO: main_window->add_accel_group(popup_menu->get_accel_group());
    }
}

void
MainGtkMenu::create_actions()
{
  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  MainWindow *main_window = gui->get_main_window();

  if (!main_window->get_action_group("app"))
    {
      action_group = Gio::SimpleActionGroup::create();
      action_group->add_action_radio_integer("mode", sigc::mem_fun(*this, &MainGtkMenu::on_menu_mode), 0);
      action_group->add_action_bool("reading", sigc::mem_fun(*this, &MainGtkMenu::on_menu_reading));
      action_group->add_action("open", sigc::mem_fun(*menus, &Menus::on_menu_open_main_window));
      action_group->add_action("restbreak", sigc::mem_fun(*menus, &Menus::on_menu_restbreak_now));
      action_group->add_action("preferences", sigc::mem_fun(*menus, &Menus::on_menu_preferences));
      action_group->add_action("exercises", sigc::mem_fun(*menus, &Menus::on_menu_exercises));
      action_group->add_action("statistics", sigc::mem_fun(*menus, &Menus::on_menu_statistics));

#ifdef HAVE_DISTRIBUTION
      action_group->add_action("network.connect", sigc::mem_fun(*menus, &Menus::on_menu_network_join));
      action_group->add_action("network.disconnect", sigc::mem_fun(*menus, &Menus::on_menu_network_leave));
      action_group->add_action("network.reconnect", sigc::mem_fun(*menus, &Menus::on_menu_network_reconnect));
      action_group->add_action_bool("network.showlog", sigc::mem_fun(*this, &MainGtkMenu::on_menu_network_log));
#endif

      action_group->add_action("about", sigc::mem_fun(*menus, &Menus::on_menu_about));
      action_group->add_action("quit", sigc::mem_fun(*menus, &Menus::on_menu_quit));

      main_window->insert_action_group("app", action_group);
    }
	
}

void
MainGtkMenu::create_menu()
{
  Glib::RefPtr<Gio::Menu> app_menu = Gio::Menu::create();

  if (show_open)
    {
      auto section = Gio::Menu::create();
      section->append(_("Open"), "open");
      app_menu->append_section(section);
    }

  auto section1 = Gio::Menu::create();
  app_menu->append_section(section1);

  auto item = Gio::MenuItem::create(_("Restbreak"), "app.restbreak");
  item->set_attribute_value("accel", Glib::Variant<Glib::ustring>::create("<Primary>r"));
  section1->append_item(item);

  section1->append(_("Exercises"), "app.exercises");
  section1->append(_("Statistics"), "app.statistics");

  auto section2 = Gio::Menu::create();
  app_menu->append_section(section2);

  auto mode_menu = Gio::Menu::create();
  item = Gio::MenuItem::create(_("Normal"), "app.mode");
  item->set_attribute_value("target",
                            Glib::Variant<int>::create(static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Normal)));
  mode_menu->append_item(item);
  item = Gio::MenuItem::create(_("Quiet"), "app.mode");
  item->set_attribute_value("target",
                            Glib::Variant<int>::create(static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Quiet)));
  mode_menu->append_item(item);
  item = Gio::MenuItem::create(_("Suspended"), "app.mode");
  item->set_attribute_value(
    "target", Glib::Variant<int>::create(static_cast<std::underlying_type_t<OperationMode>>(OperationMode::Suspended)));
  mode_menu->append_item(item);
  section2->append_submenu(_("Mode"), mode_menu);

  section2->append(_("Reading"), "app.reading");

#ifdef HAVE_DISTRIBUTION
  auto section3 = Gio::Menu::create();
  app_menu->append_section(section3);

  auto network_menu = Gio::Menu::create();

  network_menu->append(_("_Connect"), "app.network.connect");
  network_menu->append(_("_Disconnect"), "app.network.disconnect");
  network_menu->append(_("_Reconnect"), "app.network.reconnect");
  network_menu->append(_("Show _log"), "app.network.showlog");
  section3->append_submenu(_("_Network"), network_menu);
#endif

  auto section4 = Gio::Menu::create();
  app_menu->append_section(section4);

  section4->append(_("Preferences"), "app.preferences");
  section4->append(_("About"), "app.about");
  section4->append(_("Quit"), "app.quit");

#ifdef PLATFORM_OS_MACOS
  macos_popup_hack_connect(popup_menu);
#endif

  popup_menu = std::make_unique<Gtk::Menu>(app_menu);

  IGUI *gui = GUI::get_instance();
  MainWindow *main_window = gui->get_main_window();
  popup_menu->attach_to_widget(*main_window);
}

void
MainGtkMenu::popup(const guint button, const guint activate_time)
{
  (void)button;

  if (popup_menu != nullptr)
    {
      popup_menu->popup(button, activate_time);
    }
}

void
MainGtkMenu::resync(OperationMode mode, UsageMode usage, bool show_log)
{
  if (action_group)
    {
      Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("mode"))
        ->change_state(static_cast<std::underlying_type_t<OperationMode>>(mode));

      Glib::RefPtr<Gio::SimpleAction> action =
        Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("reading"));

      bool reading = (usage == UsageMode::Reading);
      action->change_state(reading);

      Glib::RefPtr<Gio::SimpleAction> log_action =
        Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("network.showlog"));
      log_action->change_state(show_log);
    }
}

void
MainGtkMenu::on_menu_mode(int mode)
{
  Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("mode"))->change_state(mode);

  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();

  switch (static_cast<OperationMode>(mode))
    {
    case OperationMode::Normal:
      menus->on_menu_normal();
      break;

    case OperationMode::Quiet:
      menus->on_menu_quiet();
      break;

    case OperationMode::Suspended:
      menus->on_menu_suspend();
      break;

    default:
      break;
    }
}

void
MainGtkMenu::on_menu_network_log()
{
  Glib::RefPtr<Gio::SimpleAction> action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("network.showlog"));

  bool active = false;
  action->get_state(active);

  active = !active;
  action->change_state(active);

  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  menus->on_menu_network_log(active);
}

void
MainGtkMenu::on_menu_reading()
{
  Glib::RefPtr<Gio::SimpleAction> action = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action_group->lookup_action("reading"));

  bool active = false;
  action->get_state(active);

  active = !active;
  action->change_state(active);

  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  menus->on_menu_reading(active);
}

#ifdef PLATFORM_OS_MACOS
// /* Taken from Gaim. needs to be gtkmm-ified. */
// /* This is a workaround for a bug in windows GTK+. Clicking outside of the
//    menu does not get rid of it, so instead we get rid of it as soon as the
//    pointer leaves the menu. */

void
MainGtkMenu::macos_popup_hack_connect(Gtk::Menu *menu)
{
  TRACE_ENTER("W32TrayMenu::macos_popup_hack_connect");

  GtkWidget *widget = (GtkWidget *)menu->gobj();
  g_signal_connect(widget, "leave-notify-event", G_CALLBACK(macos_popup_hack_leave_enter), NULL);
  g_signal_connect(widget, "enter-notify-event", G_CALLBACK(macos_popup_hack_leave_enter), NULL);

  TRACE_EXIT();
}

gboolean
MainGtkMenu::macos_popup_hack_hide(gpointer data)
{
  TRACE_ENTER("W32TrayMenu::macos_popup_hack_hide");
  if (data != NULL)
    {
      gtk_menu_popdown(GTK_MENU(data));
    }
  TRACE_EXIT();
  return FALSE;
}

gboolean
MainGtkMenu::macos_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event, void *data)
{
  TRACE_ENTER("W32TrayMenu::macos_popup_hack_leave_enter");

  TRACE_MSG(event->type << " " << event->detail);

  (void)data;
  static guint hide_docklet_timer = 0;
  if (event->type == GDK_LEAVE_NOTIFY
      /* RC: it seems gtk now generate a GDK_NOTIFY_UNKNOWN when the menu if left...*/
      && (event->detail == GDK_NOTIFY_ANCESTOR || event->detail == GDK_NOTIFY_UNKNOWN))
    {
      /* Add some slop so that the menu doesn't annoyingly disappear
         when mousing around */
      TRACE_MSG("leave " << hide_docklet_timer);
      if (hide_docklet_timer == 0)
        {
          hide_docklet_timer = g_timeout_add(500, macos_popup_hack_hide, menu);
        }
    }
  else if (event->type == GDK_ENTER_NOTIFY && event->detail == GDK_NOTIFY_ANCESTOR)
    {

      TRACE_MSG("enter " << hide_docklet_timer);

      if (hide_docklet_timer != 0)
        {
          /* Cancel the hiding if we reenter */

          g_source_remove(hide_docklet_timer);
          hide_docklet_timer = 0;
        }
    }
  TRACE_EXIT();
  return FALSE;
}
#endif
