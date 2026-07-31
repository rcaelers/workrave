// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#include "GioMenu.hh"

#include <utility>

GioMenu::Entry::~Entry()
{
  if (action != nullptr)
    {
      g_object_unref(action);
    }
}

GioMenu::GioMenu(MenuModel::Ptr menu_model)
  : menu_model(std::move(menu_model))
  , menu(g_menu_new())
  , actions(g_simple_action_group_new())
{
  workrave::utils::connect(this->menu_model->signal_update(), this, [this]() { update(); });
  update();
}

GioMenu::~GioMenu()
{
  entries.clear();
  g_object_unref(actions);
  g_object_unref(menu);
}

GMenu *
GioMenu::get_menu() const
{
  return menu;
}

GSimpleActionGroup *
GioMenu::get_actions() const
{
  return actions;
}

std::string
GioMenu::detailed_action(const std::string &name)
{
  return std::string(ACTION_PREFIX) + "." + name;
}

GMenu *
GioMenu::append_new_section(GMenu *menu)
{
  GMenu *section = g_menu_new();
  g_menu_append_section(menu, nullptr, G_MENU_MODEL(section));
  g_object_unref(section);
  return section;
}

GioMenu::EntryPtr
GioMenu::add_entry(const std::string &name, GSimpleAction *action, menus::Node::Ptr node)
{
  auto entry = std::make_shared<Entry>();
  entry->name = name;
  entry->action = action;
  entry->node = std::move(node);
  g_action_map_add_action(G_ACTION_MAP(actions), G_ACTION(action));
  entries.push_back(entry);
  return entry;
}

void
GioMenu::update()
{
  for (const auto &entry: entries)
    {
      g_action_map_remove_action(G_ACTION_MAP(actions), entry->name.c_str());
    }
  entries.clear();

  g_menu_remove_all(menu);

  GMenu *section = nullptr;
  add_children(menu, &section, menu_model->get_root());
}

void
GioMenu::add_children(GMenu *menu, GMenu **section, const menus::ContainerNode::Ptr &parent)
{
  for (const auto &child: parent->get_children())
    {
      add_node(menu, section, child);
    }
}

void
GioMenu::add_node(GMenu *menu, GMenu **section, const menus::Node::Ptr &node)
{
  if (auto n = std::dynamic_pointer_cast<menus::SeparatorNode>(node); n)
    {
      *section = nullptr;
      return;
    }

  if (auto n = std::dynamic_pointer_cast<menus::SectionNode>(node); n)
    {
      add_children(menu, section, n);
      return;
    }

  if (*section == nullptr)
    {
      *section = append_new_section(menu);
    }

  if (auto n = std::dynamic_pointer_cast<menus::SubMenuNode>(node); n)
    {
      add_submenu(*section, n);
      return;
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioGroupNode>(node); n)
    {
      add_radio_group(*section, n);
      return;
    }

  if (auto n = std::dynamic_pointer_cast<menus::ActionNode>(node); n)
    {
      add_action(*section, n);
      return;
    }

  if (auto n = std::dynamic_pointer_cast<menus::ToggleNode>(node); n)
    {
      add_toggle(*section, n);
      return;
    }

  if (auto n = std::dynamic_pointer_cast<menus::RadioNode>(node); n)
    {
      add_radio(*section, n);
      return;
    }
}

void
GioMenu::add_submenu(GMenu *section, const menus::SubMenuNode::Ptr &node)
{
  GMenu *submenu = g_menu_new();

  GMenu *sub_section = nullptr;
  add_children(submenu, &sub_section, node);

  GMenuItem *item = g_menu_item_new_submenu(node->get_dynamic_text_no_accel().c_str(), G_MENU_MODEL(submenu));
  g_menu_append_item(section, item);
  g_object_unref(item);
  g_object_unref(submenu);
}

void
GioMenu::add_action(GMenu *section, const menus::ActionNode::Ptr &node)
{
  const std::string name = node->get_id();
  GSimpleAction *action = g_simple_action_new(name.c_str(), nullptr);
  auto entry = add_entry(name, action, node);

  g_signal_connect(action,
                   "activate",
                   G_CALLBACK(+[](GSimpleAction *, GVariant *, gpointer user_data) {
                     auto *e = static_cast<Entry *>(user_data);
                     e->node->activate();
                   }),
                   entry.get());

  g_menu_append(section, node->get_dynamic_text_no_accel().c_str(), detailed_action(name).c_str());
}

void
GioMenu::add_toggle(GMenu *section, const menus::ToggleNode::Ptr &node)
{
  const std::string name = node->get_id();
  GSimpleAction *action = g_simple_action_new_stateful(name.c_str(), nullptr, g_variant_new_boolean(node->is_checked()));
  auto entry = add_entry(name, action, node);

  g_signal_connect(action,
                   "activate",
                   G_CALLBACK(+[](GSimpleAction *action, GVariant *, gpointer user_data) {
                     auto *e = static_cast<Entry *>(user_data);
                     GVariant *state = g_action_get_state(G_ACTION(action));
                     const bool checked = (g_variant_get_boolean(state) != FALSE);
                     g_variant_unref(state);
                     std::dynamic_pointer_cast<menus::ToggleNode>(e->node)->activate(!checked);
                   }),
                   entry.get());

  workrave::utils::connect(node->signal_changed(), entry, [action, node]() {
    g_simple_action_set_state(action, g_variant_new_boolean(node->is_checked()));
  });

  g_menu_append(section, node->get_dynamic_text_no_accel().c_str(), detailed_action(name).c_str());
}

void
GioMenu::add_radio_group(GMenu *section, const menus::RadioGroupNode::Ptr &node)
{
  const std::string name = node->get_id();
  GSimpleAction *action = g_simple_action_new_stateful(name.c_str(),
                                                       G_VARIANT_TYPE_INT32,
                                                       g_variant_new_int32(node->get_selected_value()));
  auto entry = add_entry(name, action, node);

  g_signal_connect(action,
                   "activate",
                   G_CALLBACK(+[](GSimpleAction *, GVariant *parameter, gpointer user_data) {
                     auto *e = static_cast<Entry *>(user_data);
                     if (parameter != nullptr)
                       {
                         std::dynamic_pointer_cast<menus::RadioGroupNode>(e->node)->activate(g_variant_get_int32(parameter));
                       }
                   }),
                   entry.get());

  workrave::utils::connect(node->signal_changed(), entry, [action, node]() {
    g_simple_action_set_state(action, g_variant_new_int32(node->get_selected_value()));
  });

  for (const auto &child: node->get_children())
    {
      add_radio(section, child);
    }
}

void
GioMenu::add_radio(GMenu *section, const menus::RadioNode::Ptr &node)
{
  GMenuItem *item = g_menu_item_new(node->get_dynamic_text_no_accel().c_str(), nullptr);
  g_menu_item_set_action_and_target_value(item,
                                          detailed_action(node->get_group_id()).c_str(),
                                          g_variant_new_int32(node->get_value()));
  g_menu_append_item(section, item);
  g_object_unref(item);
}
