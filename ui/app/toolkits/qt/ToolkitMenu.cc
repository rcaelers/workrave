// Copyright (C) 2013 - 2021 Rob Caelers <robc@krandor.nl>
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

#include <utility>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "ToolkitMenu.hh"
#include "commonui/MenuModel.hh"

using namespace detail;

ToolkitMenu::ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter)
{
  context = std::make_shared<detail::ToolkitMenuContext>(filter);
  entry = std::make_shared<ToolkitSubMenuEntry>(context, nullptr, menu_model->get_root());
  workrave::utils::connect(menu_model->signal_update(), this, [this]() { entry->init(); });
}

auto
ToolkitMenu::get_menu() const -> QMenu *
{
  return entry->get_menu();
}

ToolkitMenuContext::ToolkitMenuContext(MenuNodeFilter filter)
  : filter(std::move(filter))
{
}

auto
ToolkitMenuContext::get_filter() const -> MenuNodeFilter
{
  return filter;
}

auto
ToolkitMenuEntryFactory::create(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::Node::Ptr node)
  -> ToolkitMenuEntry::Ptr
{
  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      return std::make_shared<ToolkitSubMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      return std::make_shared<ToolkitRadioGroupMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      return std::make_shared<ToolkitActionMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      return std::make_shared<ToolkitToggleMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      return std::make_shared<ToolkitRadioMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      return std::make_shared<ToolkitSeparatorMenuEntry>(context, parent, n);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
    {
      return std::make_shared<ToolkitSectionMenuEntry>(context, parent, n);
    }

  return ToolkitMenuEntry::Ptr();
}

ToolkitMenuEntry::ToolkitMenuEntry(ToolkitMenuContext::Ptr context)
  : context(std::move(context))
{
}

auto
ToolkitMenuEntry::get_context() const -> ToolkitMenuContext::Ptr
{
  return context;
}

ToolkitSubMenuEntry::ToolkitSubMenuEntry(ToolkitMenuContext::Ptr context,
                                         ToolkitSubMenuEntry *parent,
                                         menus::SubMenuNode::Ptr node)
  : ToolkitMenuEntry(context)
  , parent(parent)
  , node(node)
{
  menu = new QMenu(tr(node->get_dynamic_text_no_accel().c_str()));
  init();
}

void
ToolkitSubMenuEntry::init()
{
  menu->clear();

  for (const auto &child_node: node->get_children())
    {
      ToolkitMenuEntry::Ptr child = ToolkitMenuEntryFactory::create(get_context(), this, child_node);
      children.push_back(child);
      auto *action = child->get_action();
      if (action != nullptr)
        {
          menu->insertAction(nullptr, action);
        }
    }
}

auto
ToolkitSubMenuEntry::get_action() const -> QAction *
{
  return menu->menuAction();
}

auto
ToolkitSubMenuEntry::get_menu() const -> QMenu *
{
  return menu;
}

//////////////////////////////////////////////////////////////////////

ToolkitRadioGroupMenuEntry::ToolkitRadioGroupMenuEntry(ToolkitMenuContext::Ptr context,
                                                       ToolkitSubMenuEntry *parent,
                                                       menus::RadioGroupNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  menu = parent->get_menu();
  for (const auto &child_node: node->get_children())
    {
      ToolkitMenuEntry::Ptr child = ToolkitMenuEntryFactory::create(get_context(), parent, child_node);
      children.push_back(child);
      menu->insertAction(nullptr, child->get_action());
    }
}

auto
ToolkitRadioGroupMenuEntry::get_action() const -> QAction *
{
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

ToolkitActionMenuEntry::ToolkitActionMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ActionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      action = new QAction(node->get_dynamic_text_no_accel().c_str(), this);
      connect(action, &QAction::triggered, [=](bool checked) { node->activate(); });
    }
}

auto
ToolkitActionMenuEntry::get_action() const -> QAction *
{
  return action;
}

//////////////////////////////////////////////////////////////////////

ToolkitToggleMenuEntry::ToolkitToggleMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ToggleNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      action = new QAction(node->get_dynamic_text_no_accel().c_str(), this);
      action->setCheckable(true);
      action->setChecked(node->is_checked());

      connect(action, &QAction::triggered, [=](bool checked) { node->activate(checked); });

      workrave::utils::connect(node->signal_changed(), this, [this, node] {
        action->setCheckable(true);
        action->setChecked(node->is_checked());
      });
    }
}

auto
ToolkitToggleMenuEntry::get_action() const -> QAction *
{
  return action;
}

//////////////////////////////////////////////////////////////////////

ToolkitRadioMenuEntry::ToolkitRadioMenuEntry(ToolkitMenuContext::Ptr context,
                                             ToolkitSubMenuEntry *parent,
                                             menus::RadioNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      action = new QAction(node->get_dynamic_text_no_accel().c_str(), this);
      action->setCheckable(true);
      action->setChecked(node->is_checked());

      connect(action, &QAction::triggered, [=](bool checked) { node->activate(); });

      workrave::utils::connect(node->signal_changed(), this, [this, node] { action->setChecked(node->is_checked()); });
    }
}

auto
ToolkitRadioMenuEntry::get_action() const -> QAction *
{
  return action;
}

//////////////////////////////////////////////////////////////////////

ToolkitSeparatorMenuEntry::ToolkitSeparatorMenuEntry(ToolkitMenuContext::Ptr context,
                                                     ToolkitSubMenuEntry *parent,
                                                     menus::SeparatorNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      action = new QAction(node->get_dynamic_text_no_accel().c_str(), this);
      action->setSeparator(true);
    }
}

auto
ToolkitSeparatorMenuEntry::get_action() const -> QAction *
{
  return action;
}

//////////////////////////////////////////////////////////////////////

ToolkitSectionMenuEntry::ToolkitSectionMenuEntry(ToolkitMenuContext::Ptr context,
                                                 ToolkitSubMenuEntry *parent,
                                                 menus::SectionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  menu = parent->get_menu();
  for (const auto &child_node: node->get_children())
    {
      auto child = ToolkitMenuEntryFactory::create(get_context(), parent, child_node);
      children.push_back(child);
      menu->insertAction(nullptr, child->get_action());
    }
}

auto
ToolkitSectionMenuEntry::get_action() const -> QAction *
{
  return nullptr;
}
