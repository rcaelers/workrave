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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "MenuModel.hh"

MenuModel::MenuModel()
{
  root = std::make_shared<MenuNode>();
}

MenuNode::Ptr
MenuModel::get_root() const
{
  return root;
}

void
MenuModel::update()
{
  update_signal();
}

boost::signals2::signal<void()> &
MenuModel::signal_update()
{
  return update_signal;
}

MenuNode::MenuNode()
    : id("workrave:root")
    , type(MenuNodeType::MENU)
{
}

MenuNode::MenuNode(const std::string &id, const std::string &text, Activated activated, MenuNodeType type)
    : id(id)
    , text(text)
    , activated(activated)
    , type(type)
{
}

MenuNode::MenuNode(MenuNodeType type)
    : type(type)
{
}

const MenuNode::MenuNodeList
MenuNode::get_menu_items() const
{
  return menu_items;
}

const std::string &
MenuNode::get_text() const
{
  return text;
}

void
MenuNode::set_text(const std::string &text)
{
  if (this->text != text)
    {
      this->text = text;
      changed_signal();
    }
}

std::string
MenuNode::get_id() const
{
  return id;
}

MenuNodeType
MenuNode::get_type() const
{
  return type;
}

bool
MenuNode::is_checked() const
{
  return checked;
}

void
MenuNode::set_checked(bool checked)
{
  if (this->checked != checked)
    {
      this->checked = checked;
      changed_signal();
    }
}

void
MenuNode::add_menu_item(MenuNode::Ptr item, MenuNode::Ptr before)
{
  MenuNodeList::iterator pos = menu_items.end();
  if (before)
    {
      pos = std::find(menu_items.begin(), menu_items.end(), before);
    }

  menu_items.insert(pos, item);
}

void
MenuNode::remove_menu_item(MenuNode::Ptr item)
{
  menu_items.remove(item);
}

void
MenuNode::activate()
{
  activated();
}

boost::signals2::signal<void()> &
MenuNode::signal_changed()
{
  return changed_signal;
}
