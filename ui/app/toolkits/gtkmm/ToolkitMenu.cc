// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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
#include "commonui/MenuModel.hh"

using namespace detail;

ToolkitMenu::ToolkitMenu(MenuModel::Ptr menu_model, MenuNodeFilter filter)
{
  menus::SubMenuNode::Ptr root = menu_model->get_root();

  auto action_group = Gio::SimpleActionGroup::create();

  context = std::make_shared<detail::ToolkitMenuContext>(action_group, filter);

  entry = std::make_shared<ToolkitSubMenuEntry>(context, nullptr, root);

  workrave::utils::connect(menu_model->signal_update(), this, [this]() { entry->init(); });
  gtk_menu = std::make_shared<Gtk::Menu>(entry->get_menu());
}

std::shared_ptr<Gtk::Menu>
ToolkitMenu::get_menu() const
{
  return gtk_menu;
}

Glib::RefPtr<Gio::SimpleActionGroup>
ToolkitMenu::get_action_group() const
{
  return context->get_action_group();
}

ToolkitMenuContext::ToolkitMenuContext(Glib::RefPtr<Gio::SimpleActionGroup> action_group, MenuNodeFilter filter)
  : action_group(action_group)
  , filter(filter)
{
}

Glib::RefPtr<Gio::SimpleActionGroup>
ToolkitMenuContext::get_action_group() const
{
  return action_group;
}

MenuNodeFilter
ToolkitMenuContext::get_filter() const
{
  return filter;
}

ToolkitMenuEntry::Ptr
ToolkitMenuEntryFactory::create(ToolkitMenuContext::Ptr context, ToolkitSubMenuEntry *parent, menus::Node::Ptr node)
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

  return {};
}

ToolkitMenuEntry::ToolkitMenuEntry(ToolkitMenuContext::Ptr context)
  : context(context)
{
}

ToolkitMenuContext::Ptr
ToolkitMenuEntry::get_context() const
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
  init();
}

void
ToolkitSubMenuEntry::init()
{
  if (parent != nullptr)
    {
      const MenuNodeFilter &filter = get_context()->get_filter();
      if (!filter || filter(node))
        {
          menu = Gio::Menu::create();
          auto item = Gio::MenuItem::create(node->get_text(), std::string("app.") + node->get_id());
          item->set_submenu(menu);
          parent->add(item);
        }
    }
  else
    {
      if (!menu)
        {
          menu = Gio::Menu::create();
        }
      menu->remove_all();
    }

  add_section();
  for (auto menu_to_add: node->get_children())
    {
      ToolkitMenuEntry::Ptr child = ToolkitMenuEntryFactory::create(get_context(), this, menu_to_add);
      children.push_back(child);
    }
}

Glib::RefPtr<Gio::Menu>
ToolkitSubMenuEntry::get_menu() const
{
  return menu;
};

void
ToolkitSubMenuEntry::add(Glib::RefPtr<Gio::MenuItem> item)
{
  if (current_section)
    {
      current_section->append_item(item);
    }
};

void
ToolkitSubMenuEntry::add_section()
{
  if (menu)
    {
      current_section = Gio::Menu::create();
      menu->append_section(current_section);
    }
};

//////////////////////////////////////////////////////////////////////

ToolkitRadioGroupMenuEntry::ToolkitRadioGroupMenuEntry(ToolkitMenuContext::Ptr context,
                                                       ToolkitSubMenuEntry *parent,
                                                       menus::RadioGroupNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  auto action = context->get_action_group()->add_action_radio_integer(
    node->get_id(),
    [node](int value) { node->activate(value); },
    0);
  action->change_state(node->get_selected_value());

  workrave::utils::connect(node->signal_changed(), this, [action, node]() { action->change_state(node->get_selected_value()); });

  for (auto child_node: node->get_children())
    {
      auto child = std::make_shared<ToolkitRadioMenuEntry>(context, parent, child_node);
      children.push_back(child);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitActionMenuEntry::ToolkitActionMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ActionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  action = context->get_action_group()->add_action(node->get_id(), [node]() { node->activate(); });

  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_id());
      parent->add(item);
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitToggleMenuEntry::ToolkitToggleMenuEntry(ToolkitMenuContext::Ptr context,
                                               ToolkitSubMenuEntry *parent,
                                               menus::ToggleNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  action = context->get_action_group()->add_action_bool(node->get_id(), [this, node]() {
    bool active = false;
    action->get_state(active);
    node->activate(!active);
  });

  action->change_state(node->is_checked());

  const MenuNodeFilter &filter = get_context()->get_filter();
  if (!filter || filter(node))
    {
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_id());
      workrave::utils::connect(node->signal_changed(), this, [this, node]() { action->change_state(node->is_checked()); });
      parent->add(item);
    }
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
      auto item = Gio::MenuItem::create(node->get_dynamic_text(), std::string("app.") + node->get_group_id());
      item->set_attribute_value("target", Glib::Variant<int>::create(node->get_value()));
      parent->add(item);
    }
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
      parent->add_section();
    }
}

//////////////////////////////////////////////////////////////////////

ToolkitSectionMenuEntry::ToolkitSectionMenuEntry(ToolkitMenuContext::Ptr context,
                                                 ToolkitSubMenuEntry *parent,
                                                 menus::SectionNode::Ptr node)
  : ToolkitMenuEntry(context)
{
  for (auto child_node: node->get_children())
    {
      auto child = ToolkitMenuEntryFactory::create(context, parent, child_node);
      children.push_back(child);
    }
}
