// Menus.cc
//
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef MENUITEM_HH
#define MENUITEM_HH

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

enum class MenuItemType : int
{
    MENU,
    ACTION,
    CHECK,
    RADIO,
};

class MenuItem
{
public:
  typedef boost::shared_ptr<MenuItem> Ptr;
  typedef std::list<MenuItem::Ptr> MenuItemList;
  typedef boost::function<void ()> Activated;
  
  static Ptr create();
  static Ptr create(const std::string &text, Activated activated, MenuItemType type = MenuItemType::ACTION);
  
  MenuItem();
  MenuItem(const std::string &text, Activated activated, MenuItemType type = MenuItemType::ACTION);

  const std::string &get_text() const;
  void set_text(const std::string &text);

  MenuItemType get_type() const;

  bool is_checked() const;
  void set_checked(bool checked);
  
  const MenuItemList get_submenus() const;
  void add_menu(MenuItem::Ptr submenu, MenuItem::Ptr before = MenuItem::Ptr());
  void remove_menu(MenuItem::Ptr submenu);
  
  void activate();
  
  boost::signals2::signal<void()> &signal_changed();
  boost::signals2::signal<void(MenuItem::Ptr item, MenuItem::Ptr before)> &signal_added();
  boost::signals2::signal<void(MenuItem::Ptr item)> &signal_removed();
  
private:
  std::string text;
  Activated activated;
  MenuItemType type;
  MenuItemList submenus;
  bool checked;
  boost::signals2::signal<void()> changed_signal;
  boost::signals2::signal<void(MenuItem::Ptr item, MenuItem::Ptr before)> added_signal;
  boost::signals2::signal<void(MenuItem::Ptr item)> removed_signal;
};
#endif // MENUITEM_HH
