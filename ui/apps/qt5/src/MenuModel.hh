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

#ifndef MENUMODEL_HH
#define MENUMODEL_HH

#include <string>
#include <list>
#include <functional>
#include <memory>
#include <boost/signals2.hpp>

enum class MenuNodeType : int
{
    MENU,
    SEPARATOR,
    ACTION,
    CHECK,
    RADIO,
};


class MenuNode
{
public:
  typedef std::shared_ptr<MenuNode> Ptr;
  typedef std::list<MenuNode::Ptr> MenuNodeList;
  typedef std::function<void ()> Activated;

  MenuNode();
  MenuNode(const std::string &id, const std::string &text, Activated activated, MenuNodeType type = MenuNodeType::ACTION);
  MenuNode(MenuNodeType type);

  const std::string &get_text() const;
  void set_text(const std::string &text);

  std::string get_id() const;
  MenuNodeType get_type() const;

  bool is_checked() const;
  void set_checked(bool checked);

  const MenuNodeList get_menu_items() const;
  void add_menu_item(MenuNode::Ptr submenu, MenuNode::Ptr before = MenuNode::Ptr());
  void remove_menu_item(MenuNode::Ptr submenu);

  void activate();

  boost::signals2::signal<void()> &signal_changed();

private:
  const std::string id;
  std::string text;
  Activated activated;
  MenuNodeType type;
  MenuNodeList menu_items;
  bool checked { false };
  boost::signals2::signal<void()> changed_signal;
};

class MenuModel
{
public:
  typedef std::shared_ptr<MenuModel> Ptr;

  MenuModel();

  MenuNode::Ptr get_root() const;
  void update();

  boost::signals2::signal<void()> &signal_update();

private:
  MenuNode::Ptr root;
  boost::signals2::signal<void()> update_signal;
};

#endif // MENUMODEL_HH
