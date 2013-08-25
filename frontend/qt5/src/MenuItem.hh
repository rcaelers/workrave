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

#include <QMenu>

#include "Bitmask.hh"

enum class MenuItemFlag : int
{
    NONE = 0,
    TOPMENU = 1,
    SUBMENU = 2,
    CHECK = 4,
    RADIO = 8,
    ACTIVE = 16,
};
WR_BITMASK(MenuItemFlag)

class MenuItem
{
public:
  typedef boost::function<void ()> Activated;
  typedef boost::shared_ptr<MenuItem> Ptr;
  typedef std::list<MenuItem::Ptr> MenuItemList;
  
  static Ptr create();
  static Ptr create(const std::string &text, Activated activated, MenuItemFlag flags = MenuItemFlag::NONE);
  
  MenuItem();
  MenuItem(const std::string &text, Activated activated, MenuItemFlag flags = MenuItemFlag::NONE);

  const std::string &get_text() const;
  void set_text(const std::string &text);

  MenuItemFlag get_flags() const;
  void set_flags(MenuItemFlag flags);

  const MenuItemList get_submenus() const;
  void add_menu(MenuItem::Ptr submenu);

  void activate();
  
  boost::signals2::signal<void()> &signal_changed();
  boost::signals2::signal<void(MenuItem::Ptr item)> &signal_added();
  
private:
  std::string text;
  Activated activated;
  MenuItemFlag flags;
  MenuItemList submenus;
  boost::signals2::signal<void()> changed_signal;
  boost::signals2::signal<void(MenuItem::Ptr item)> added_signal;
};
#endif // MENUITEM_HH
