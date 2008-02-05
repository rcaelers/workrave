// MainGtkMenu.hh --- Menu using Gtk+
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
// $Id: MainGtkMenu.hh 1436 2008-02-03 18:03:23Z rcaelers $
//

#ifndef MAINGTKMENU_HH
#define MAINGTKMENU_HH

#include "config.h"

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/uimanager.h>

#include "Menu.hh"

class MainGtkMenu : public Menu
{
public:
  MainGtkMenu(bool show_open);
  virtual ~MainGtkMenu();

  void add_stock_item(const Glib::RefPtr<Gtk::IconFactory> &factory,
                      const std::string &path,
                      const Glib::ustring &id,
                      const Glib::ustring &label);
  
  void register_stock_items();

  virtual void create_actions();
  virtual void create_ui();
  virtual void post_init() {}

  virtual void init();
  virtual void popup(const guint button, const guint activate_time);
  virtual void resync(workrave::OperationMode mode, bool show_log);

private:
  void on_menu_network_log();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
  
protected:
  Glib::RefPtr<Gtk::UIManager> ui_manager;
  Glib::RefPtr<Gtk::ActionGroup> action_group;

  //!
  Gtk::Menu *popup_menu;

  //!
  bool show_open;
};

#endif // MAINGTKMENU_HH
