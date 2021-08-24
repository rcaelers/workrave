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

#include <iostream>

#include "MenuModel.hh"

MenuModel::MenuModel()
{
  root = std::make_shared<menus::SubMenuNode>();
}

menus::SubMenuNode::Ptr
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

menus::Node::Node()
  : id("workrave:root")
{
}

menus::Node::Node(std::string_view id, const std::string &text, Activated activated)
  : id(id)
  , text(text)
  , activated(activated)
{
}

std::string
menus::Node::get_id() const
{
  return id;
}

std::string
menus::Node::get_text() const
{
  return text;
}

bool
menus::Node::is_visible() const
{
  return visible;
}

void
menus::Node::set_text(const std::string &text)
{
  if (this->text != text)
    {
      this->text = text;
    }
}

void
menus::Node::set_visible(bool visible)
{
  if (this->visible != visible)
    {
      this->visible = visible;
      changed_signal();
    }
}

boost::signals2::signal<void()> &
menus::Node::signal_changed()
{
  return changed_signal;
}

void
menus::Node::activate()
{
  activated();
}

menus::SubMenuNode::Ptr
menus::SubMenuNode::create(std::string_view id, const std::string &text)
{
  return std::make_shared<menus::SubMenuNode>(id, text);
}

menus::SubMenuNode::SubMenuNode(std::string_view id, const std::string &text)
  : Node(id, text)
{
}

void
menus::SubMenuNode::add(menus::Node::Ptr item, menus::Node::Ptr before)
{
  auto pos = children.end();
  if (before)
    {
      pos = std::find(children.begin(), children.end(), before);
    }

  children.insert(pos, item);
}

void
menus::SubMenuNode::remove(menus::Node::Ptr item)
{
  children.remove(item);
}

const std::list<menus::Node::Ptr>
menus::SubMenuNode::get_children() const
{
  return children;
}

menus::ActionNode::Ptr
menus::ActionNode::create(std::string_view id, const std::string &text, Activated activated)
{
  return std::make_shared<menus::ActionNode>(id, text, activated);
}

menus::ActionNode::ActionNode(std::string_view id, const std::string &text, Activated activated)
  : Node(id, text, activated)
{
}

menus::ToggleNode::Ptr
menus::ToggleNode::create(std::string_view id, const std::string &text, Activated activated)
{
  return std::make_shared<menus::ToggleNode>(id, text, activated);
}

menus::ToggleNode::ToggleNode(std::string_view id, const std::string &text, Activated activated)
  : Node(id, text, activated)
{
}

bool
menus::ToggleNode::is_checked() const
{
  return checked;
}

void
menus::ToggleNode::set_checked(bool checked)
{
  if (this->checked != checked)
    {
      this->checked = checked;
      changed_signal();
    }
}

void
menus::ToggleNode::activate()
{
  set_checked(!is_checked());
  if (activated)
    {
      activated();
    }
}

void
menus::ToggleNode::activate(bool state)
{
  set_checked(state);
  if (activated)
    {
      activated();
    }
}

menus::RadioNode::Ptr
menus::RadioNode::create(std::shared_ptr<RadioGroupNode> group,
                         std::string_view id,
                         const std::string &text,
                         int value,
                         Activated activated)
{
  return std::make_shared<menus::RadioNode>(group, id, text, value, activated);
}

menus::RadioNode::RadioNode(std::shared_ptr<RadioGroupNode> group,
                            std::string_view id,
                            const std::string &text,
                            int value,
                            Activated activated)
  : Node(id, text, activated)
  , value(value)
  , group(group)
{
}

int
menus::RadioNode::get_value() const
{
  return value;
}

void
menus::RadioNode::set_value(int value)
{
  if (this->value != value)
    {
      this->value = value;
      changed_signal();
    }
}

bool
menus::RadioNode::is_checked() const
{
  return checked;
}

void
menus::RadioNode::set_checked(bool checked)
{
  if (this->checked != checked)
    {
      this->checked = checked;

      if (checked)
        {
          std::shared_ptr<RadioGroupNode> rg = group.lock();
          if (rg)
            {
              rg->select(get_id());
            }
        }
      changed_signal();
    }
}

std::string
menus::RadioNode::get_group_id() const
{
  std::shared_ptr<RadioGroupNode> g = group.lock();
  if (g)
    {
      return g->get_id();
    }
  else
    {
      return "";
    };
}

void
menus::RadioNode::activate()
{
  set_checked(true);
  if (activated)
    {
      activated();
    }
}

menus::RadioGroupNode::Ptr
menus::RadioGroupNode::create(std::string_view id, const std::string &text, Activated activated)
{
  return std::make_shared<menus::RadioGroupNode>(id, text, activated);
}

menus::RadioGroupNode::RadioGroupNode(std::string_view id, const std::string &text, Activated activated)
  : Node(id, text, activated)
{
}

void
menus::RadioGroupNode::add(menus::RadioNode::Ptr item, menus::RadioNode::Ptr before)
{
  auto pos = children.end();
  if (before)
    {
      pos = std::find(children.begin(), children.end(), before);
    }

  children.insert(pos, item);
}

void
menus::RadioGroupNode::remove(menus::RadioNode::Ptr item)
{
  children.remove(item);
}

const std::list<menus::RadioNode::Ptr>
menus::RadioGroupNode::get_children() const
{
  return children;
}

menus::RadioNode::Ptr
menus::RadioGroupNode::get_selected_node() const
{
  auto i = std::find_if(children.begin(), children.end(), [this](auto n) { return n->get_id() == selected_id; });
  if (i != children.end())
    {
      return (*i);
    }
  return menus::RadioNode::Ptr();
}

int
menus::RadioGroupNode::get_selected_value() const
{
  auto node = get_selected_node();
  if (node)
    {
      return node->get_value();
    }

  return 0;
}

std::string
menus::RadioGroupNode::get_selected_id() const
{
  return selected_id;
}

void
menus::RadioGroupNode::select(std::string id)
{
  if (selected_id == id)
    {
      return;
    }
  selected_id = id;

  for (auto &node: children)
    {
      node->set_checked(node->get_id() == id);
    }
  changed_signal();
}

void
menus::RadioGroupNode::select(int value)
{
  bool changed{false};
  for (auto &node: children)
    {
      if (node->get_value() == value)
        {
          if (!node->is_checked())
            {
              selected_id = node->get_id();
              node->set_checked(true);
              changed = true;
            }
        }
      else
        {
          if (node->is_checked())
            {
              node->set_checked(false);
              changed = true;
            }
        }
    }
  if (changed)
    {
      changed_signal();
    }
}

void
menus::RadioGroupNode::activate()
{
  auto node = get_selected_node();
  if (node)
    {
      node->activate();
    }
  if (activated)
    {
      activated();
    }
}

void
menus::RadioGroupNode::activate(int value)
{
  auto i = std::find_if(children.begin(), children.end(), [value](auto n) { return n->get_value() == value; });
  if (i != children.end())
    {
      auto node = *i;
      node->activate();
    }

  if (activated)
    {
      activated();
    }
}

menus::SeparatorNode::Ptr
menus::SeparatorNode::create()
{
  return std::make_shared<menus::SeparatorNode>();
}
