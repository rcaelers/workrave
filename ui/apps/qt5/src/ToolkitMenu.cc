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

#include "ToolkitMenu.hh"
#include "MenuModel.hh"

ToolkitMenu::ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter)
{
  MenuNode::Ptr root = menu_model->get_root();

  menu = std::make_shared<SubMenuEntry>(root, filter);
  connections.connect(menu_model->signal_update(), std::bind(&ToolkitMenu::on_update, this));
}

QMenu *
ToolkitMenu::get_menu() const
{
  return menu->get_menu();
}

void
ToolkitMenu::on_update()
{
  menu->init();
}

MenuEntry::Ptr
MenuEntryFactory::create(MenuNode::Ptr menu_node, MenuNodeFilter filter)
{
  switch (menu_node->get_type())
    {
    case MenuNodeType::MENU:
      return std::make_shared<SubMenuEntry>(menu_node, filter);

    case MenuNodeType::SEPARATOR:
      return std::make_shared<SeperatorMenuEntry>(menu_node, filter);

    case MenuNodeType::ACTION:
    case MenuNodeType::CHECK:
    case MenuNodeType::RADIO:
      return std::make_shared<ActionMenuEntry>(menu_node, filter);
    }

  return MenuEntry::Ptr();
}

MenuEntry::MenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter)
    : menu_node(menu_node)
    , filter(filter)
{
}

MenuNode::Ptr
MenuEntry::get_menu_node() const
{
  return menu_node;
}

MenuNodeFilter
MenuEntry::get_filter() const
{
  return filter;
}

SubMenuEntry::SubMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter)
    : MenuEntry(menu_node, filter)
{
  menu = new QMenu(tr(menu_node->get_text().c_str()));
  init();
}

void
SubMenuEntry::init()
{
  MenuNode::Ptr menu_node = get_menu_node();
  MenuNodeFilter filter = get_filter();

  menu->clear();
  for (const MenuNode::Ptr &menu_to_add : get_menu_node()->get_menu_items())
    {
      const MenuNodeFilter &filter = get_filter();

      if (!filter || filter(menu_to_add))
        {
          MenuEntry::Ptr child = MenuEntryFactory::create(menu_to_add, filter);
          children.push_back(child);
          menu->insertAction(nullptr, child->get_action());
        }
    }
  connections.connect(get_menu_node()->signal_changed(), [this]() { menu->setTitle(get_menu_node()->get_text().c_str()); });
}

QAction *
SubMenuEntry::get_action() const
{
  return menu->menuAction();
}

QMenu *
SubMenuEntry::get_menu() const
{
  return menu;
}

ActionMenuEntry::ActionMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter)
    : MenuEntry(menu_node, filter)
{
  connections.connect(menu_node->signal_changed(), std::bind(&ActionMenuEntry::on_menu_changed, this));

  action = new QAction(menu_node->get_text().c_str(), this);
  action->setCheckable(menu_node->get_type() == MenuNodeType::RADIO || menu_node->get_type() == MenuNodeType::CHECK);
  action->setChecked(menu_node->is_checked());

  connect(action, &QAction::triggered, this, &ActionMenuEntry::on_action);
}

QAction *
ActionMenuEntry::get_action() const
{
  return action;
}

void
ActionMenuEntry::on_menu_changed()
{
  if (action != nullptr)
    {
      action->setText(get_menu_node()->get_text().c_str());
      action->setCheckable(get_menu_node()->get_type() == MenuNodeType::RADIO || get_menu_node()->get_type() == MenuNodeType::CHECK);
      action->setChecked(get_menu_node()->is_checked());
    }
}

void
ActionMenuEntry::on_action(bool checked)
{
  get_menu_node()->set_checked(checked);
  get_menu_node()->activate();
}

SeperatorMenuEntry::SeperatorMenuEntry(MenuNode::Ptr menu_node, MenuNodeFilter filter)
    : MenuEntry(menu_node, filter)
{
  action = new QAction(menu_node->get_text().c_str(), this);
  action->setSeparator(true);
}

QAction *
SeperatorMenuEntry::get_action() const
{
  return action;
}
