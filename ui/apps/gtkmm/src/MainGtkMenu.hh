// Copyright (C) 2001 - 2009, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef MAINGTKMENU_HH
#define MAINGTKMENU_HH

#include <string>

#include <gtkmm.h>

#include "MenuBase.hh"

class MainGtkMenu : public MenuBase
{
public:
  MainGtkMenu(bool show_open);
  ~MainGtkMenu() override = default;

  void add_stock_item(const Glib::RefPtr<Gtk::IconFactory> &factory,
                      const std::string &path,
                      const Glib::ustring &id,
                      const Glib::ustring &label);

  void register_stock_items();

  virtual void create_actions();
  virtual void create_menu();
  virtual void post_init()
  {
  }

  void init() override;
  void popup(const guint button, const guint activate_time) override;
  void resync(workrave::OperationMode mode, workrave::UsageMode usage, bool show_log) override;

private:
  void on_menu_network_log();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
  void on_menu_reading();

#ifdef PLATFORM_OS_MACOS
  void macos_popup_hack_connect(Gtk::Menu *menu);
  static gboolean macos_popup_hack_hide(gpointer data);
  static gboolean macos_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event, void *data);
#endif

protected:
  Glib::RefPtr<Gtk::UIManager> ui_manager;
  Glib::RefPtr<Gtk::ActionGroup> action_group;

  //!
  Gtk::Menu *popup_menu{nullptr};

  //!
  bool show_open{false};
};

#endif // MAINGTKMENU_HH
