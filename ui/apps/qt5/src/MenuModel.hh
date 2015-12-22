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
#include <list>
#include <functional>
#include <memory>
#include <boost/signals2.hpp>

enum class MenuModelType : int
{
    MENU,
    ACTION,
    CHECK,
    RADIO,
};

class MenuModel
{
public:
  typedef std::shared_ptr<MenuModel> Ptr;
  typedef std::list<MenuModel::Ptr> MenuModelList;
  typedef std::function<void ()> Activated;

  MenuModel();
  MenuModel(const std::string &id, const std::string &text, Activated activated, MenuModelType type = MenuModelType::ACTION);

  const std::string &get_text() const;
  void set_text(const std::string &text);

  std::string get_id() const;
  MenuModelType get_type() const;

  bool is_checked() const;
  void set_checked(bool checked);

  const MenuModelList get_submenus() const;
  void add_menu(MenuModel::Ptr submenu, MenuModel::Ptr before = MenuModel::Ptr());
  void remove_menu(MenuModel::Ptr submenu);

  void activate();

  boost::signals2::signal<void()> &signal_changed();
  boost::signals2::signal<void(MenuModel::Ptr item, MenuModel::Ptr before)> &signal_added();
  boost::signals2::signal<void(MenuModel::Ptr item)> &signal_removed();

private:
  const std::string id;
  std::string text;
  Activated activated;
  MenuModelType type;
  MenuModelList submenus;
  bool checked;
  boost::signals2::signal<void()> changed_signal;
  boost::signals2::signal<void(MenuModel::Ptr item, MenuModel::Ptr before)> added_signal;
  boost::signals2::signal<void(MenuModel::Ptr item)> removed_signal;
};

#endif // MENUITEM_HH
